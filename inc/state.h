#ifndef STATE_H
#define STATE_H
#include <linux/limits.h>
#include "errorcode.h"
#include "player.h" // IWYU pragma: keep
#include <stc/cstr.h>

#define T vec_player, player_t*
#include <stc/vec.h>
/* A "state" defines a context in jamb, which is a prefix directory, players,
 * and a config file. open_state(dir) reads or opens the context in either the
 * current directory (if called with NULL), or the given directory. It also
 * loads all players from /etc/jamb/players and the given directory/players
 * save_state() syncs all pending operations. reopen_state() reopens a 
 * previously closed but not deallocated state */
typedef struct {
	vec_player p_players;
	enum {
		STATE_STATUS_UNINITIALIZED,
		STATE_STATUS_CLOSED,
		STATE_STATUS_OPEN,
	} status;
	char wd[PATH_MAX];
} state;

extern state global_state;

// operates on the global state if NULL is passed 
jamb_status_t open_state(const char *, state*);
// operates on the global state if NULL is passed 
jamb_status_t reopen_state(state*);
// operates on the global state if NULL is passed 
jamb_status_t save_state(state*);
// operates on the global state if NULL is passed 
jamb_status_t close_state(state*);

// ^ copied four times for IDEs

#endif
