#ifndef PLAYER_H
#define PLAYER_H
#include "metadata.h"
#include "stream.h"

#include <linux/limits.h>
/* A player is a type that holds one loaded player and it's associated stream. */
typedef struct {
	stream_t stream; // a player can only probably play one file at a time?
	jamb_status_t (*start)(void* this);
	int (*score)(void* this, metadata);
	metadata (*get_real_metadata)(void* this);

	enum {
		PLAYER_SCRIPT,
		PLAYER_C,
	} type;

	union {
		struct {
			char dir[PATH_MAX];
		} script;

		struct {
			void *handle;
			void *state;
		} c;
	} as;

	char name[PATH_MAX];
} player_t;

jamb_status_t script_start             (void*);
jamb_status_t script_setup             (void*, metadata);
jamb_status_t script_cleanup           (void*);
jamb_status_t script_play              (void*);
jamb_status_t script_pause             (void*);
jamb_status_t script_seek              (void*, cursor_t);
cursor_t      script_get_cursor        (void*);
int           script_score             (void*, metadata);
metadata      script_get_real_metadata (void*);

typedef int      (*c_start)             (void*);
typedef int      (*c_score)             (void*, const metadata);
typedef int      (*c_setup)             (void*, const metadata);
typedef int      (*c_cleanup)           (void*);
typedef int      (*c_play)              (void*);
typedef int      (*c_pause)             (void*);
typedef int      (*c_seek)              (void*, const cursor_t);
typedef cursor_t (*c_get_cursor)        (void*, const cursor_t);
typedef metadata (*c_get_real_metadata) (void*, const cursor_t);

jamb_status_t player_destruct(player_t*);

#endif
