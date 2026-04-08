#include "state.h"
#include "errorcode.h"
#define LOG_H_ENUM_PREFIX_
#define LOG_H_NAMESPACE_ 
#include "logging.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

state global_state = {
	.status = STATE_STATUS_UNINITIALIZED
};

jamb_status_t open_state(const char *dir, state* s)
{
	if(s->status != STATE_STATUS_UNINITIALIZED)
		return JAMB_LOGIC_ERROR;

	if(!s)
		s = &global_state;

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
	
	if(snprintf(NULL, 0, "%s/jamb.db", s->wd) >= sizeof(s->wd))
		return JAMB_BAD_PATH;

	strcat(s->wd, "/jamb.db");

	int status = sqlite3_open(s->wd, &s->db);
	
	logf_out("Opened state: %s", LOG_DBG, s->wd);

	*strrchr(s->wd, '/') = 0;

	if(status != SQLITE_OK){
		logf_out("Database at %s/jamb.db could not be opened!",
				LOG_WARN, s->wd);
		return JAMB_BACKEND_ERROR;
	}

	s->status = STATE_STATUS_OPEN;

	return JAMB_OK;
}

jamb_status_t reopen_state(state* s)
{
	if(s->status != STATE_STATUS_CLOSED)
		return JAMB_LOGIC_ERROR;

	int status = sqlite3_open(s->wd, &s->db);

	if(status != SQLITE_OK)
		return JAMB_BACKEND_ERROR;

	s->status = STATE_STATUS_OPEN;

	return JAMB_OK;
}

jamb_status_t save_state(state* s)
{
	return JAMB_OK; // currently a no-op
}


jamb_status_t close_state(state* s)
{
	sqlite3_close(s->db);
	s->status = STATE_STATUS_CLOSED;
	return JAMB_OK;
}
