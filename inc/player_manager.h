#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H
#include "errorcode.h"
#include "player.h"
#include "state.h"

// Loads players from /etc/jamb and the state's directory into the state
jamb_status_t load_players(state*);

// Gets the best player from the global state given some metadata
jamb_status_t get_best_player(player_t **player, const metadata);

// Gets the best player from the given state given some metadata
jamb_status_t get_best_player_ex(const state* s, player_t **player, const metadata);
#endif
