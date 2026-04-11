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
#include <dirent.h>
#include <errno.h>
#define LOG_H_ENUM_PREFIX_
#define LOG_H_NAMESPACE_ 
#include "logging.h"

void add_c_player(const char *dir)
{
	
}

jamb_status_t add_player(const char *dir, const char *name, state *s)
{
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
		*strrchr(new->as.script.dir, '/') = 0;
		if(strlen(new->as.script.dir) + strlen("/player") 
				>= PATH_MAX)
			return JAMB_BAD_PATH;
		strcat(new->as.script.dir, "/player");
	}
	else if(strcmp("C", type_str) == 0){
		new->type = PLAYER_C;
		*strrchr(new->as.script.dir, '/') = 0;
		if(strlen(new->as.script.dir) + strlen("/player.so") 
				>= PATH_MAX)
			return JAMB_BAD_PATH;
		strcat(new->as.script.dir, "/player.so");
		assert(0);
	}
	else{
		logf_out("Value of 'type' is neither SCRIPT nor C",
				LOG_WARN, dir);
		free(new);
		free(file_str);
		json_object_put(obj);
		return JAMB_INVALID_PLAYER;
	}

	vec_player_push(&s->p_players, new);

	free(file_str);
	json_object_put(obj);
	return JAMB_OK;
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
				logf_out("Could not add player %s"
					 "(%s)", LOG_WARN,
					 direntp->d_name, 
					 strjamberror(res));
			}

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
