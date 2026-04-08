#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include "errorcode.h"
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
	gboolean init = FALSE;
	gboolean verbose = FALSE;
	gboolean test = FALSE;
	gchar *prefix = NULL;
	gchar *watched = NULL;
	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &version, 
			"Show version information", NULL },
		{ "init", 'i', 0, G_OPTION_ARG_NONE, &init, 
			"Initialize jamb", NULL },
		{ "prefix", 'p', 0, G_OPTION_ARG_STRING, &prefix, 
			"Set a prefix for jamb's database", NULL },
		{ "watch-dir", 'd', 0, G_OPTION_ARG_STRING, &watched, 
			"Add a directory to jamb's watched directories", NULL },
		{ "verbose", '\0', 0, G_OPTION_ARG_NONE, &verbose, 
			"Make jamb verbose", NULL },
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
	
	if(verbose == TRUE){
		set_log_level(LOG_TRACE);
	}
	else{
		set_log_level(LOG_INFO);
	}

	if(test){
		int max_errorcode = 0;
#define X(name) max_errorcode++;
		JAMB_ERRORCODES
#undef X
		log_out("Errorcodes:", LOG_DBG);
		for(int i = 0; i < max_errorcode; i++){
			logf_out("%02d: %s", LOG_DBG, i, strjamberror(i));
		}
	}

	g_option_context_free(ctx);

	if(version == TRUE){
		printf("%s\n", PACKAGE_VERSION_STR);
		return EXIT_SUCCESS;
	}

	if(init == TRUE){
		jamb_status_t res;
		if((res = open_state(prefix, &global_state)) != JAMB_OK){
			logf_out("Could not read state! (open_state() returned %s)\n", 
					LOG_ERROR, strjamberror(res));
		}
	}

	if(watched){
		switch(global_state.status){
			case STATE_STATUS_UNINITIALIZED: 
				if(open_state(prefix, &global_state))
					return 1;
				break;
			case STATE_STATUS_CLOSED:
				if(reopen_state(&global_state))
					return 1;
				break;
			case STATE_STATUS_OPEN:
				log_out("Not reopening open state", LOG_DBG);
				break;
		}
		
		logf_out("global state wd: %s", LOG_DBG, global_state.wd);

		jamb_status_t res = load_players(&global_state);
		if(res != JAMB_OK){
			logf_out("Failed loading players! (%s)", LOG_WARN,
					strjamberror(res));
		}
	}
	close_state(&global_state);
	return EXIT_SUCCESS;
}
