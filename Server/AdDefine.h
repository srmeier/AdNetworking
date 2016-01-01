#ifndef __ADDEFINE_H_
#define __ADDEFINE_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

//-----------------------------------------------------------------------------
#define MAX_PACKET 0xFF
#define MAX_SOCKETS 0x10

//-----------------------------------------------------------------------------
#define WOOD_WAIT_TIME 3000

//-----------------------------------------------------------------------------
#define FLAG_QUIT 0x0000
#define FLAG_WOOD_QUEST 0x0011
#define FLAG_WOOD_UPDATE 0x0010
#define FLAG_WOOD_GETTIME 0x0012

#define FLAG_PLAYER_POS 0x0020
#define FLAG_PLAYER_CONNECT 0x0021
#define FLAG_PLAYER_DISCONNECT 0x0022

#endif
