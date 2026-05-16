#ifndef STREAM_H
#define STREAM_H
#include <stdint.h>
#include "errorcode.h"
#include "metadata.h"
#define STREAM_SAMPLES_PER_SECOND 60
typedef uint64_t cursor_t;

/* A stream is anything that is pausable, playable, and seekable. The type
 * stream_t defines an abstract interface for defining such streams, and any
 * streams that require more data must set it up via the setup and cleanup
 * functions, optionally with the provided space in stream_t*/
typedef struct abstract_stream_t {
	jamb_status_t (*setup)(void* this, metadata);
	jamb_status_t (*cleanup)(void* this);
	jamb_status_t (*play)(void* this);
	jamb_status_t (*pause)(void* this);
	jamb_status_t (*seek)(void* this, cursor_t);
	cursor_t      (*get_cursor)(void* this);
} abstract_stream_t;

typedef struct {
	void* p_data;
	abstract_stream_t s;
} stream_t;
#endif
