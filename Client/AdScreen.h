#ifndef __ADSCREEN_H_
#define __ADSCREEN_H_

#include "AdBase.h"

//-----------------------------------------------------------------------------
class AdScreen: public AdBase {
private:
	static int      s_iFrames;
	static float    s_fTotTime;
	static uint64_t s_uiLastCount;
	static uint64_t s_uiDiffCount;
	static uint64_t s_uiCountFreq;
	static float    s_fElapsedTime;
	static uint64_t s_uiCurrentCount;

private:
	static SDL_Surface* s_pScreen;
	static SDL_Texture* s_pTexture;

public:
	static bool Init(void);
	static void Quit(void);

public:
	static void Clear(void);
	static void Present(void);
	static void DrawSprite(
		SDL_Point pntLoc, SDL_Surface* pSprite
	);
};

#endif
