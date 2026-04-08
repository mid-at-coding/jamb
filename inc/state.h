#ifndef STATE_H
#define STATE_H
#include <linux/limits.h>
#include "sqlite3.h"
#include "errorcode.h"
/* A "state" defines a context in jamb, which is currently a prefix directory
 * and a config file. open_state(dir) reads or opens the database in either the
 * current directory (if called with NULL), or the given directory. 
 * save_state() syncs all pending operations. reopen_state() reopens a 
 * previously closed but not deallocated state */
typedef struct {
	enum {
		STATE_STATUS_UNINITIALIZED,
		STATE_STATUS_CLOSED,
		STATE_STATUS_OPEN,
	} status;
	sqlite3 *db;
	char wd[PATH_MAX];
} state;

extern state global_state;

jamb_status_t open_state(const char *, state*);
jamb_status_t reopen_state(state*);
jamb_status_t save_state(state*);
jamb_status_t close_state(state*);

#endif
