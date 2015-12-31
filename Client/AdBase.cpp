/*
*/

#include "AdBase.h"
#include "AdScreen.h"
#include "AdSpriteManager.h"

//-----------------------------------------------------------------------------
SDL_Window*   AdBase::s_pWindow;
SDL_Renderer* AdBase::s_pRenderer;

int AdBase::s_iWinScale;
int AdBase::s_iWinWidth;
int AdBase::s_iWinHeight;

//-----------------------------------------------------------------------------
bool AdBase::Init(int iWidth, int iHeight, int iScale) {
	s_iWinScale  = iScale;
	s_iWinWidth  = iWidth;
	s_iWinHeight = iHeight;

	srand((unsigned int) time(NULL));

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "ER: %s\n", SDL_GetError());
		return false;
	}

	if(TTF_Init() == -1) {
		fprintf(stderr, "ER: %s\n", TTF_GetError());
		return false;
	}

	int flags = IMG_INIT_JPG|IMG_INIT_PNG;
	if((IMG_Init(flags)&flags) != flags) {
		fprintf(stderr, "ER: %s\n", IMG_GetError());
		return false;
	}

	flags = MIX_INIT_OGG|MIX_INIT_MP3;
	if((Mix_Init(flags)&flags) != flags) {
		fprintf(stderr, "ER: %s\n", Mix_GetError());
		return false;
	}

	if(SDLNet_Init() == -1) {
		fprintf(stderr, "ER: %s\n", SDLNet_GetError());
		return false;
	}

	s_pWindow = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		s_iWinScale*s_iWinWidth,
		s_iWinScale*s_iWinHeight,
		0
	);

	if(s_pWindow == NULL) {
		fprintf(stderr, "ER: %s\n", SDL_GetError());
		return false;
	}

	s_pRenderer = SDL_CreateRenderer(
		s_pWindow, -1,
		SDL_RENDERER_ACCELERATED|
		SDL_RENDERER_PRESENTVSYNC
	);

	if(s_pRenderer == NULL) {
		fprintf(stderr, "ER: %s\n", SDL_GetError());
		return false;
	}

	if(AdScreen::Init() == false) return false;
	if(AdSpriteManager::Init(8, 8) == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
void AdBase::Quit(void) {
	AdSpriteManager::Quit();
	AdScreen::Quit();

	SDL_DestroyRenderer(s_pRenderer);
	s_pRenderer = NULL;

	SDL_DestroyWindow(s_pWindow);
	s_pWindow = NULL;

	SDLNet_Quit();

	Mix_Quit();

	IMG_Quit();

	TTF_Quit();

	SDL_Quit();
}
