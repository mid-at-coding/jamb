#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include "errorcode.h"
#include "player.h"
#include "player_manager.h"
#include "state.h"
#define LOG_H_IMPLEMENTATION
#define LOG_H_ENUM_PREFIX_
#define LOG_H_NAMESPACE_ 
#include "logging.h"
#define STR(x) #x
#define EXPAND_STR(x) STR(x)
#define PACKAGE_VERSION_STR EXPAND_STR(PACKAGE_VERSION)

int main(int argc, char **argv)
{
	gboolean version = FALSE;
	gboolean verbose = FALSE;
	gboolean vverbose = FALSE;
	gboolean test = FALSE;
	gchar *prefix = NULL;
	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &version, 
			"Show version information", NULL },
		{ "prefix", 'p', 0, G_OPTION_ARG_STRING, &prefix, 
			"Set a prefix for jamb's database", NULL },
		{ "verbose", '\0', 0, G_OPTION_ARG_NONE, &verbose, 
			"Make jamb verbose", NULL },
		{ "vverbose", '\0', 0, G_OPTION_ARG_NONE, &vverbose, 
			"Make jamb very verbose", NULL },
		{ "test", 't', 0, G_OPTION_ARG_NONE, &test, 
			"Run jamb's self-test", NULL },
		{ NULL }
	};

	GError* error = NULL;
	GOptionContext* ctx;

	ctx = g_option_context_new(NULL);
	g_option_context_add_main_entries(ctx, entries, NULL);

	if(!g_option_context_parse(ctx, &argc, &argv, &error)){
		g_print("%s\n", error->message);
		g_option_context_free(ctx);
		g_error_free(error);
		return EXIT_FAILURE;
	}
	
	g_option_context_free(ctx);

	if(verbose == TRUE){
		set_log_level(LOG_DBG);
	}
	else if(vverbose == TRUE){
		set_log_level(LOG_TRACE);
	}
	else{
		set_log_level(LOG_INFO);
	}

	if(version == TRUE){
		printf("%s\n", PACKAGE_VERSION_STR);
		return EXIT_SUCCESS;
	}

	jamb_status_t res;
	if((res = open_state(prefix, NULL)) != JAMB_OK){
		logf_out("Could not read state! (open_state() returned %s)\n", 
				LOG_ERROR, strjamberror(res));
	}
	logf_out("global state wd: %s", LOG_DBG, global_state.wd);

	if(test){
		player_t *player;
		get_best_player(&player, "");
		if(player){
			logf_out("Player.start() returned %s",
					LOG_DBG,
					strjamberror(player->start(player)));
		}
		else{
			log_out("Could not get an appropriate player!",
					LOG_TRACE);
		}
		player_destruct(player);
	}

	save_state(NULL);
	close_state(NULL);
	return EXIT_SUCCESS;
}
