#include "errorcode.h"
#include <string.h>
#define LOG_H_ENUM_PREFIX_
#define LOG_H_NAMESPACE_ 
#include "logging.h"

char *strjamberror(jamb_status_t s)
{
	switch(s){
#define X(name) case name: return #name;
		JAMB_ERRORCODES
#undef X
	}
}

#define STR(x) #x
#define EXPAND_STR(x) STR(x)
jamb_status_t jamberrorstr(const char *err){
#define X(name) if(strcmp(EXPAND_STR(name), err) == 0) return name;
	JAMB_ERRORCODES
#undef X
	logf_out("Given string %s is not a real status code!", LOG_WARN,
			err);
	return JAMB_BACKEND_ERROR;
}
