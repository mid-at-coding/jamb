#include "state.h"
#include "errorcode.h"
#define LOG_H_ENUM_PREFIX_
#define LOG_H_NAMESPACE_ 
#include "logging.h"
#include <unistd.h>
#include <string.h>
#include "player_manager.h"

state global_state = {
	.status = STATE_STATUS_UNINITIALIZED
};

jamb_status_t open_state(const char *dir, state* s)
{
	if(!s)
		s = &global_state;

	if(s->status != STATE_STATUS_UNINITIALIZED)
		return JAMB_LOGIC_ERROR;

	if(!dir){
		if(!getcwd(s->wd, sizeof(s->wd))){
			return JAMB_UNSPECIFIED;
		}
		logf_out("CWD: %s", LOG_TRACE,
				s->wd);
	}
	else{
		if(strlen(dir) + 1 >= sizeof(s->wd))
			return JAMB_BAD_PATH;

		strcpy(s->wd, dir);
	}
	
	logf_out("Reading state from %s", LOG_TRACE,
			s->wd);
	
	s->status = STATE_STATUS_OPEN;

	jamb_status_t res = load_players(s);
	if(res != JAMB_OK){
		logf_out("Failed loading players! (%s)", LOG_WARN,
				strjamberror(res));
	}

	return JAMB_OK;
}

jamb_status_t reopen_state(state* s)
{
	if(!s)
		s = &global_state;

	if(s->status != STATE_STATUS_CLOSED)
		return JAMB_LOGIC_ERROR;


	s->status = STATE_STATUS_OPEN;

	return JAMB_OK;
}

jamb_status_t save_state(state* s)
{
	if(!s)
		s = &global_state;

	return JAMB_OK; // currently a no-op
}


jamb_status_t close_state(state* s)
{
	if(!s)
		s = &global_state;

	s->status = STATE_STATUS_CLOSED;
	return JAMB_OK;
}
