/*
*/

#include "AdScreen.h"

//-----------------------------------------------------------------------------
int      AdScreen::s_iFrames;
float    AdScreen::s_fTotTime;
uint64_t AdScreen::s_uiLastCount;
uint64_t AdScreen::s_uiDiffCount;
uint64_t AdScreen::s_uiCountFreq;
float    AdScreen::s_fElapsedTime;
uint64_t AdScreen::s_uiCurrentCount;

SDL_Surface* AdScreen::s_pScreen;
SDL_Texture* AdScreen::s_pTexture;

//-----------------------------------------------------------------------------
bool AdScreen::Init(void) {
	s_pScreen = SDL_CreateRGBSurface(
		0, s_iWinWidth, s_iWinHeight,
		32, 0x00, 0x00, 0x00, 0x00
	);

	if(s_pScreen == NULL) {
		fprintf(stderr, "ER: %s\n", SDL_GetError());
		return false;
	}

	s_pTexture = SDL_CreateTexture(
		GetRenderer(),
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		s_pScreen->w,
		s_pScreen->h
	);

	if(s_pTexture == NULL) {
		fprintf(stderr, "ER: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void AdScreen::Quit(void) {
	SDL_DestroyTexture(s_pTexture);
	s_pTexture = NULL;

	SDL_FreeSurface(s_pScreen);
	s_pScreen = NULL;
}

//-----------------------------------------------------------------------------
void AdScreen::Clear(void) {
	SDL_RenderClear(GetRenderer());
	SDL_FillRect(s_pScreen, NULL, 0x00);
}

//-----------------------------------------------------------------------------
void AdScreen::Present(void) {
	int pitch;
	void* pixels;

	SDL_LockTexture(s_pTexture, NULL, &pixels, &pitch);

	SDL_ConvertPixels(
		s_pScreen->w,
		s_pScreen->h,
		s_pScreen->format->format,
		s_pScreen->pixels,
		s_pScreen->pitch,
		SDL_PIXELFORMAT_RGBA8888,
		pixels, pitch
	);

	SDL_UnlockTexture(s_pTexture);

	SDL_RenderCopy(GetRenderer(), s_pTexture, NULL, NULL);
	SDL_RenderPresent(GetRenderer());

	s_iFrames++;

	s_uiCurrentCount = SDL_GetPerformanceCounter();
	s_uiDiffCount    = (s_uiCurrentCount-s_uiLastCount);
	s_uiCountFreq    = SDL_GetPerformanceFrequency();

	s_fElapsedTime = ((float) s_uiDiffCount/(float) s_uiCountFreq);

	if((1.0f/60.0f)-s_fElapsedTime > 0) {
		// NOTE: without this there seems to be a more consistent framerate
		// but on older computers sometimes the vsync doesn't work right

		//SDL_Delay((uint32_t) (1000.0f*((1.0f/60.0f)-s_fElapsedTime)));

		s_fTotTime += 1.0f/60.0f;
	} else {
		s_fTotTime += s_fElapsedTime;
	}

	s_uiLastCount = s_uiCurrentCount;

	if(s_fTotTime >= 1.0f) {
		char strTitle[NAME_LENGTH] = "";
		sprintf(strTitle, "%s, FPS: %d", WINDOW_TITLE, s_iFrames);

		SDL_SetWindowTitle(GetWindow(), strTitle);

		s_fTotTime  = 0;
		s_iFrames = 0;
	}
}

//-----------------------------------------------------------------------------
void AdScreen::DrawSprite(SDL_Point pntLoc, SDL_Surface* pSprite) {
	if(pSprite == NULL) return;

	SDL_Rect rect = {pntLoc.x, pntLoc.y, pSprite->w, pSprite->h};
	SDL_BlitSurface(pSprite, NULL, s_pScreen, &rect);
}
