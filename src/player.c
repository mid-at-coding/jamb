#include "player.h"
#include "errorcode.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#define LOG_H_ENUM_PREFIX_
#define LOG_H_NAMESPACE_ 
#include "logging.h"

// TODO factor

static int val(char input){
	if(input >= '0' && input <= '9')
		return input - '0';
	if(isalpha(input))
		return tolower(input) - 'a' + 10;
	return -1;
}

static bool isdigit_radix(const char input, const unsigned int radix){
	int res = val(input);
	if(res == -1)
		return false;
	return (unsigned)res < radix;
}

typedef struct {
	long res;
	int status;
} atoi_res_ll;

static atoi_res_ll safe_atoi_ll(const char *input, const int radix){
	atoi_res_ll res = {0};
	if(radix < 2 || radix > 36){ // UB
		res.status = EINVAL;
		return res;
	}
	errno = 0; // strtol need not set errno to 0 on a sucessful call, so zero-initialize errno
	res.res = strtoll(input, NULL, radix); // use strtoll because atoll need not set errno on failure
	if(res.res == LLONG_MAX  || res.res == LLONG_MIN){ // in this case, a conforming implementation must set errno to ERANGE, but check anyway
		res.status = ERANGE;
	}
	else if(errno){ // technically a conforming implementation may only set errno to ERANGE, but check for any error
		res.status = errno;
		errno = 0;
	}
	else if(!res.res){ // either our number is zero, or our input is whitespace followed by a non-digit number
		// check if the first non space character in input isn't a number
		for(const char *c = input; *c; c++){
			if(isspace(*c)) // the leading whitespace must be specified by isspace (7.22.1.4 #2)
				continue;
			if(!isdigit_radix(*c, radix)) // custom function that's written to specs defined in 7.22.1.4 #3
				res.status = EINVAL;
		}
	}
	return res;
}

/* creates a pipe, runs a command, copies the output, and frees the command */
static char *read_command(player_t *player, char *cmd)
{
	FILE *pipe = popen(cmd, "r");
	if(!pipe){
		free(cmd);
		return NULL;
	}
	
	char res[1024];

	if(!fgets(res, sizeof(res), pipe)){
		log_out("Script returned nothing!", LOG_WARN);
		pclose(pipe);
		free(cmd);
		return NULL;
	}

	size_t len = strlen(res) + 1;
	char *str = malloc(len);
	strcpy(str, res);
	pclose(pipe);
	free(cmd);
	return str;
}

/* creates a pipe, runs a command, collects a status code, and frees the 
 * command and pipe */
static jamb_status_t run_command(player_t *player, char *cmd)
{
	FILE *pipe = popen(cmd, "r");
	if(!pipe){
		free(cmd);
		return JAMB_BACKEND_ERROR;
	}
	
	char res[1024];

	if(!fgets(res, sizeof(res), pipe)){
		log_out("Script returned nothing, assuming JAMB_OK",
				LOG_WARN);
		pclose(pipe);
		free(cmd);
		return JAMB_OK;
	}

	pclose(pipe);
	free(cmd);
	return jamberrorstr(res);
}

jamb_status_t script_setup(void* this, metadata m)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);

	size_t len = snprintf(NULL, 0, "./%s setup %s", 
			th->as.script.dir, m);
	char *command = malloc(len + 1);
	snprintf(command, len, "./%s setup %s",
			th->as.script.dir, m);
	if(!command)
		return JAMB_NO_MEMORY;

	return run_command(this, command);
}

jamb_status_t script_cleanup(void* this)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);

	size_t len = snprintf(NULL, 0, "./%s cleanup", 
			th->as.script.dir);
	char *command = malloc(len + 1);
	snprintf(command, len, "./%s cleanup",
			th->as.script.dir);
	if(!command)
		return JAMB_NO_MEMORY;

	return run_command(this, command);
}

jamb_status_t script_play(void* this)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);

	size_t len = snprintf(NULL, 0, "./%s play", 
			th->as.script.dir);
	char *command = malloc(len + 1);
	snprintf(command, len, "./%s play",
			th->as.script.dir);
	if(!command)
		return JAMB_NO_MEMORY;

	return run_command(this, command);
}

jamb_status_t script_pause(void* this)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);

	size_t len = snprintf(NULL, 0, "./%s pause", 
			th->as.script.dir);
	char *command = malloc(len + 1);
	snprintf(command, len, "./%s pause",
			th->as.script.dir);
	if(!command)
		return JAMB_NO_MEMORY;

	return run_command(this, command);
}

jamb_status_t script_seek(void* this, cursor_t cursor)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);

	size_t len = snprintf(NULL, 0, "./%s seek %ld", 
			th->as.script.dir, cursor);
	char *command = malloc(len + 1);
	snprintf(command, len, "./%s seek %ld",
			th->as.script.dir, cursor);
	if(!command)
		return JAMB_NO_MEMORY;

	return run_command(this, command);
}

cursor_t script_get_cursor(void* this)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);

	size_t len = snprintf(NULL, 0, "./%s get_cursor", 
			th->as.script.dir);
	char *command = malloc(len + 1);
	snprintf(command, len, "./%s get_cursor",
			th->as.script.dir);
	if(!command)
		return -1;

	char *returned = read_command(this, command);
	atoi_res_ll r = safe_atoi_ll(returned, 10);
	if(r.status){
		logf_out("Could not parse %s as a number!", LOG_WARN,
				returned);
		return -1;
	}
	free(returned);

	return r.res;
}

int script_score(void* this, metadata m)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);

	size_t len = snprintf(NULL, 0, "./%s get_cursor", 
			th->as.script.dir);
	char *command = malloc(len + 1);
	snprintf(command, len, "./%s get_cursor",
			th->as.script.dir);
	if(!command)
		return -1;

	char *returned = read_command(this, command);
	atoi_res_ll r = safe_atoi_ll(returned, 10);
	if(r.status){
		logf_out("Could not parse %s as a number!", LOG_WARN,
				returned);
		return -1;
	}
	free(returned);

	return r.res;
}

char *script_get_real_metadata(void* this)
{
	player_t *th = this;
	assert(th->type == PLAYER_SCRIPT);
	assert(0);
	return 0;
}
