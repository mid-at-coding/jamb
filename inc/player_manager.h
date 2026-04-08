#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H
#include "errorcode.h"
#include "player.h"
#include "state.h"
#include <stc/cstr.h>

#define T vec_player, player_t*
#include <stc/vec.h>

extern vec_player p_players;

jamb_status_t load_players(state*);
jamb_status_t get_best_player(player_t *player, const char *metadata);
#endif
