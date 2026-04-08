#ifndef ERRORCODE_H
#define ERRORCODE_H
typedef enum {
#define JAMB_ERRORCODES\
	X(JAMB_OK) \
	X(JAMB_INCORRECT_TYPE) \
	X(JAMB_INCORRECT_FILE_TYPE) \
	X(JAMB_BAD_PATH) \
	X(JAMB_NONEXISTENT) \
	X(JAMB_NO_MEMORY) \
	X(JAMB_LOGIC_ERROR) \
	X(JAMB_NO_PERMISSIONS) \
	X(JAMB_UNSPECIFIED) \
	X(JAMB_AMBIGUOUS) \
	X(JAMB_NOTHING_APPROPRIATE) \
	X(JAMB_BOUNDS_ERROR) \
	X(JAMB_INVALID_PLAYER) \
	X(JAMB_BACKEND_ERROR)
#define X(name) name,
	JAMB_ERRORCODES
#undef X
} jamb_status_t;

char *strjamberror(jamb_status_t);
jamb_status_t jamberrorstr(const char*);
#endif
