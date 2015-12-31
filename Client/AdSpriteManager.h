#ifndef __ADSPRITEMANAGER_H_
#define __ADSPRITEMANAGER_H_

#include "AdBase.h"

//-----------------------------------------------------------------------------
class AdSpriteManager {
private:
	static TTF_Font*     s_pFont;
	static SDL_Surface** s_pTiles;
	static int           s_iNumTile;

protected:
	static int s_iSprWidth;
	static int s_iSprHeight;

public:
	static bool Init(int iWidth, int iHeight);
	static void Quit(void);

public:
	static SDL_Surface* BuildSprite(int iWidth, int iHeight, int pIndices[]);
	static SDL_Surface* BuildSprite(SDL_Color sdlColor, const char* pString, ...);
};

#endif
