#include "../../inc/errorcode.h"
#include "../../inc/stream.h"
#include <stdio.h>

extern int start(void* dat)
{
	return JAMB_OK;
}

extern int score(void* dat, const char * const metadata)
{
	printf("Hello, World!\n");
	return -1;
}

extern int setup(void* dat, const char * const metadata)
{
	return JAMB_OK;
}

extern int cleanup(void* dat)
{
	return JAMB_OK;
}

extern int play(void* dat)
{
	return JAMB_OK;
}

extern int pause(void* dat)
{
	return JAMB_OK;
}

extern int seek(void* dat, const cursor_t)
{
	return JAMB_OK;
}

extern cursor_t get_cursor(void* dat)
{
	return UINT64_MAX - 1;
}

extern char * get_real_metadata(void* dat)
{
	return "";
}

int main()
{
	return 0;
}
