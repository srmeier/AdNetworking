#ifndef __ADBASE_H_
#define __ADBASE_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "Duktape.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_net.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"

#define UP_DIREC          0
#define DOWN_DIREC        1
#define LEFT_DIREC        2
#define RIGHT_DIREC       3
#define COLLISION_LAYER   2
#define PLAYER_DRAW_LAYER 0
#define NAME_LENGTH       0xFF
#define WINDOW_TITLE      "Prototype"
#define MAP_LOCATION      "data/maps/%s.json"
#define FONT_LOCATION     "data/SDS_8x8.ttf"
#define SPRITE_LOCATION   "data/spritesheet.bmp"

//-----------------------------------------------------------------------------
#define MAX_PACKET 0xFF
#define MAX_SOCKETS 0x10

#define FLAG_QUIT 0x0000
#define FLAG_WOOD_QUEST 0x0011
#define FLAG_WOOD_UPDATE 0x0010
#define FLAG_WOOD_GETTIME 0x0012

#define FLAG_PLAYER_POS 0x0020
#define FLAG_PLAYER_CONNECT 0x0021
#define FLAG_PLAYER_DISCONNECT 0x0022

//-----------------------------------------------------------------------------
class AdBase {
private:
	static SDL_Window*   s_pWindow;
	static SDL_Renderer* s_pRenderer;

protected:
	static int s_iWinScale;
	static int s_iWinWidth;
	static int s_iWinHeight;

protected:
	static SDL_Window*   GetWindow(void)   {return s_pWindow;}
	static SDL_Renderer* GetRenderer(void) {return s_pRenderer;}

public:
	static bool Init(int iWidth, int iHeight, int iScale);
	static void Quit(void);

public:
	static int GetScale(void)  {return s_iWinScale;}
	static int GetWidth(void)  {return s_iWinWidth;}
	static int GetHeight(void) {return s_iWinHeight;}
};

#endif
