#ifndef PLAYER_H
#define PLAYER_H
#include "metadata.h"
#include "stream.h"

#include <linux/limits.h>
/* A player is a type that holds one loaded player and it's associated stream. */
typedef struct {
	stream_t stream; // a player can only probably play one file at a time?
	jamb_status_t (*start)(void*);
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
			int placeholder;
		} c;
	} as;

	char name[PATH_MAX];
} player_t;

jamb_status_t script_start(void*);
jamb_status_t script_setup(void*, metadata);
jamb_status_t script_cleanup(void*);
jamb_status_t script_play(void*);
jamb_status_t script_pause(void*);
jamb_status_t script_seek(void*, cursor_t);
cursor_t      script_get_cursor(void*);
int           script_score(void*, metadata);
metadata      script_get_real_metadata(void*);

#endif
