#include "player_manager.h"
#include "json_object.h"
#include "json_tokener.h"
#include "player.h"
#include "errorcode.h"
#include "state.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <linux/limits.h>
#include <stc/common.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#define LOG_H_ENUM_PREFIX_
#define LOG_H_NAMESPACE_ 
#include "logging.h"

static jamb_status_t add_c_player(const char *c_dir, player_t *new)
{
	logf_out("Adding C player at path %s", LOG_TRACE, c_dir);
	new->as.c.handle = dlopen(c_dir, RTLD_NOW);
	if(!new->as.c.handle){
		logf_out("dlopen failed on %s", LOG_WARN, c_dir);
		struct stat sb;
		if(stat(c_dir, &sb) && errno == ENOENT){
			logf_out("Note: %s does not exist", LOG_WARN,
					c_dir);
		}
		return JAMB_INVALID_PLAYER;
	}

	new->type = PLAYER_C;
	new->get_real_metadata   = dlsym(new->as.c.handle, "get_real_metadata");
	new->score               = dlsym(new->as.c.handle, "score");
	new->start               = dlsym(new->as.c.handle, "start");
	new->stream.s.setup      = dlsym(new->as.c.handle, "setup");
	new->stream.s.cleanup    = dlsym(new->as.c.handle, "cleanup");
	new->stream.s.play       = dlsym(new->as.c.handle, "play");
	new->stream.s.pause      = dlsym(new->as.c.handle, "pause");
	new->stream.s.seek       = dlsym(new->as.c.handle, "seek");
	new->stream.s.get_cursor = dlsym(new->as.c.handle, "pause");

	// check if all symbols are non-NULL
	// NOTE: there may be some situations where a NULL is allowed, but I don't
	// think they will really come up
	if(new->get_real_metadata &&
	   new->score &&
	   new->start &&
	   new->stream.s.setup &&
	   new->stream.s.cleanup &&
	   new->stream.s.play &&
	   new->stream.s.pause &&
	   new->stream.s.seek &&
	   new->stream.s.get_cursor)
		return JAMB_OK;

	logf_out("One or more expected symbols in %s are null", LOG_WARN, c_dir);

	return JAMB_INVALID_PLAYER;
}

jamb_status_t add_player(const char *dir, const char *name, state *s)
{
	jamb_status_t return_value = JAMB_OK;
	errno = 0;
	FILE *fp = fopen(dir, "r");
	if(!fp){
		if(errno == EINVAL)
			return JAMB_LOGIC_ERROR;
		if(errno == EACCES)
			return JAMB_NO_PERMISSIONS;
		return JAMB_UNSPECIFIED;
	}
	
	errno = 0;
	if(fseek(fp, 0L, SEEK_END))
		return JAMB_UNSPECIFIED;
	
	errno = 0;
	size_t size = ftell(fp);
	if(errno)
		return JAMB_UNSPECIFIED;

	errno = 0;
	rewind(fp);

	char *file_str = malloc(size);

	if(!file_str)
		return JAMB_NO_MEMORY;

	if(size != fread(file_str, 1, size, fp))
		return JAMB_UNSPECIFIED;

	json_object *obj = json_tokener_parse(file_str);

	json_object *type_obj;
	if(!json_object_object_get_ex(obj, "type", &type_obj)){
		logf_out("Json file %s does not have a key 'type'",
				LOG_WARN, dir);
		return JAMB_INVALID_PLAYER;
	}

	const char *type_str = json_object_get_string(type_obj);

	if(!type_str){
		logf_out("Value of 'type' is not string", LOG_WARN, dir);
		return JAMB_INVALID_PLAYER;
	}

	player_t *new = malloc(sizeof(player_t));

	if(!new)
		return JAMB_NO_MEMORY;

	strcpy(new->name, name);
	if(strcmp("SCRIPT", type_str) == 0){
		new->type = PLAYER_SCRIPT;
		new->get_real_metadata   = script_get_real_metadata;
		new->score               = script_score;
		new->start               = script_start;
		new->stream.s.setup      = script_setup;
		new->stream.s.cleanup    = script_cleanup;
		new->stream.s.play       = script_play;
		new->stream.s.pause      = script_pause;
		new->stream.s.seek       = script_seek;
		new->stream.s.get_cursor = script_get_cursor;

		strcpy(new->as.script.dir, dir);
		
		// I'm actually not sure if it is more correct to check for this or 
		// not -- can a valid player path have no /?
		if(strrchr(new->as.script.dir, '/'))
			*strrchr(new->as.script.dir, '/') = 0;

		if(strlen(new->as.script.dir) + strlen("/player") >= PATH_MAX){
			return_value = JAMB_BAD_PATH;
			goto early_exit;
		}

		strcat(new->as.script.dir, "/player");
	}
	else if(strcmp("C", type_str) == 0){
		new->type = PLAYER_C;

		char *c_dir = malloc(strlen(dir) + 1);

		if(!c_dir){
			return_value = JAMB_NO_MEMORY;
			goto early_exit;
		}

		strcpy(c_dir, dir);

		if(strrchr(c_dir, '/'))
			*strrchr(c_dir, '/') = 0;

		if(strlen(c_dir) + strlen("/player.so") >= PATH_MAX){
			return_value = JAMB_BAD_PATH;
			goto early_exit;
		}

		strcat(c_dir, "/player.so");
		
		if((return_value = add_c_player(c_dir, new)) 
				!= JAMB_OK){
			goto early_exit;
		}
	}
	else{
		logf_out("Value of 'type' is neither SCRIPT nor C",
				LOG_WARN, dir);

		return_value = JAMB_INVALID_PLAYER;
		goto early_exit;
	}

	vec_player_push(&s->p_players, new);

early_exit:
	if(return_value != JAMB_OK)
		free(new);
	free(file_str);
	json_object_put(obj);
	return return_value;
}

jamb_status_t load_players_from_path(const char *wd, state *s)
	// TODO check stat calls for errors
	// TODO refactor
{
	struct dirent *direntp = NULL;
	DIR *dirp = NULL;
	size_t path_len;

	path_len = strlen(wd);

	if(!path_len || (path_len >= PATH_MAX)){
		logf_out("Loading players with a bad path (%s)", LOG_WARN,
				wd);
		return JAMB_BAD_PATH;
	}
	errno = 0;
	dirp = opendir(wd);
	logf_out("Opening directory %s", LOG_TRACE, wd);

	if(!dirp){
		logf_out("Directory %s could not be opened!",
				LOG_WARN, wd);
		if(errno == EACCES)
			return JAMB_NO_PERMISSIONS;
		if(errno == ENOENT || errno == ENAMETOOLONG
		   || errno == ENOTDIR)
			return JAMB_BAD_PATH;
		if(errno == ENOMEM)
			return JAMB_NO_MEMORY;
		return JAMB_UNSPECIFIED;
	}

	while((direntp = readdir(dirp)) != NULL){ // check out all the
						  // directories in wd
		struct stat fstat;
		char full_name[PATH_MAX];

		if((path_len + strlen(direntp->d_name) + 1) > PATH_MAX){
			logf_out("Potential player path %s%s is too large,"
				"skipping", LOG_WARN, wd, direntp->d_name);
			continue;
		}

		if(!(strcmp(".", direntp->d_name) &&
		     strcmp("..",direntp->d_name)))
			continue;
		
		strcpy(full_name, wd);
		if(full_name[path_len - 1] != '/')
			strcat(full_name, "/");
		strcat(full_name, direntp->d_name);
		if(full_name[strlen(full_name) - 1] != '/')
			strcat(full_name, "/");

		if(stat(full_name, &fstat) < 0)
			continue;

		if(S_ISDIR(fstat.st_mode)){
			// check that the config file does indeed exist
			if((strlen(full_name) + strlen("conf.json") + 1)
			>= PATH_MAX){
				logf_out("Player config path %s%s is too large,"
					"skipping"
					, LOG_WARN, full_name, "conf.json");
				continue;
			}
			strcat(full_name, "conf.json");

			if(stat(full_name, &fstat) < 0){
				logf_out("Rejecting potential player %s "
					 "because %s doesn't exist",
					LOG_TRACE, direntp->d_name, full_name);
				continue;
			}

			if(!S_ISREG(fstat.st_mode)){
				logf_out("Rejecting potential player %s "
					 "because %s is not a file",
					 LOG_TRACE, direntp->d_name, full_name);
				continue;
			}

			logf_out("Adding player %s", LOG_DBG,
					direntp->d_name);
			
			jamb_status_t res;
			if((res = add_player(full_name, direntp->d_name, s))){
				logf_out("Could not add player %s "
					 "(%s)", LOG_WARN,
					 direntp->d_name, 
					 strjamberror(res));
			}
			// TODO call player->start?

		}
	}

	closedir(dirp);
	return JAMB_OK;
}

jamb_status_t load_players(state *s)
{
	static bool loaded = false;
	s->p_players = vec_player_init();
	if(loaded){
		log_out("Loading players again!", LOG_WARN);
		return JAMB_LOGIC_ERROR;
	}
	jamb_status_t res = load_players_from_path("/etc/jamb/players", s);
	if(res)
		return res;

	char wd_players[PATH_MAX];
	// no need to do error checking because load_players_from_path will
	// tell us when it's wrong
	snprintf(wd_players, PATH_MAX, "%s/players", s->wd); 
	res = load_players_from_path(wd_players, s);
	if(res)
		return res;
	loaded = true;
	return JAMB_OK;
}

jamb_status_t get_best_player(player_t **player, const metadata m)
{
	return get_best_player_ex(&global_state, player, m);
}

jamb_status_t get_best_player_ex(const state* s, player_t **player, const metadata m)
	// TODO: add some way to prioritize certain players
{
	player_t *max_player;
	int max_res = -1;
	int curr_res;
	for(c_each(curr, vec_player, s->p_players)){
		curr_res = (*curr.ref)->score((*curr.ref), m);
		logf_out("Player %s returned %d", LOG_TRACE,
				(*curr.ref)->name, curr_res);
		if(curr_res == 1){ // we found a full match, exit early
			*player = (*curr.ref);
			return JAMB_OK;
		}
		if(curr_res > max_res){
			max_player = (*curr.ref);
			max_res = curr_res;
		}
	}

	if(max_res > -1){
		*player = max_player;
		logf_out("Best player (%s) is not an exact match!",
				LOG_TRACE, max_player->name);
		return JAMB_OK;
	}
	*player = NULL;
	logf_out("Could not find an appropriate player!", LOG_DBG);
	return JAMB_NOTHING_APPROPRIATE;
}
