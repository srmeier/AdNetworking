/*
*/

#include "AdBase.h"
#include "AdSpriteManager.h"

//-----------------------------------------------------------------------------
TTF_Font*     AdSpriteManager::s_pFont;
SDL_Surface** AdSpriteManager::s_pTiles;
int           AdSpriteManager::s_iNumTile;

int AdSpriteManager::s_iSprWidth;
int AdSpriteManager::s_iSprHeight;

//-----------------------------------------------------------------------------
bool AdSpriteManager::Init(int iWidth, int iHeight) {
	s_iSprWidth  = iWidth;
	s_iSprHeight = iHeight;

	s_pFont = TTF_OpenFont(FONT_LOCATION, 8);
	if(s_pFont == NULL) {
		fprintf(stderr, "ER: %s\n", TTF_GetError());
		return false;
	}

	SDL_Surface* surface = SDL_LoadBMP(SPRITE_LOCATION);
	if(surface == NULL) {
		fprintf(stderr, "ER: %s\n", SDL_GetError());
		return false;
	}

	s_iNumTile = (surface->w/s_iSprWidth)*(surface->h/s_iSprHeight)+1;
	s_pTiles = (SDL_Surface**) malloc(s_iNumTile*sizeof(SDL_Surface*));

	int i, x, y;
	SDL_Rect rect = {0, 0, s_iSprWidth, s_iSprHeight};
	for(i=0; i<s_iNumTile; i++) {
		s_pTiles[i] = SDL_CreateRGBSurface(
			0, s_iSprWidth, s_iSprHeight, 24,
			0x00, 0x00, 0x00, 0x00
		);

		if(i==0) {
			SDL_FillRect(s_pTiles[i], NULL, 0xFF00FF);
		} else {
			x = (i-1)%(surface->w/s_iSprWidth);
			y = (i-x)/(surface->w/s_iSprWidth);
			rect.x = s_iSprWidth*x, rect.y = s_iSprHeight*y;
			SDL_BlitSurface(surface, &rect, s_pTiles[i], NULL);
		}
	}

	SDL_FreeSurface(surface);
	surface = NULL;

	return true;
}

//-----------------------------------------------------------------------------
void AdSpriteManager::Quit(void) {
	int i;
	for(i=0; i<s_iNumTile; i++) {
		SDL_FreeSurface(s_pTiles[i]);
		s_pTiles[i] = NULL;
	}

	free(s_pTiles);
	s_pTiles = NULL;

	TTF_CloseFont(s_pFont);
	s_pFont = NULL;
}

//-----------------------------------------------------------------------------
SDL_Surface* AdSpriteManager::BuildSprite(int iWidth, int iHeight, int pIndices[]) {
	SDL_Surface* pSprite;
	pSprite = SDL_CreateRGBSurface(
		0, s_iSprWidth*iWidth, s_iSprHeight*iHeight,
		24, 0x00, 0x00, 0x00, 0x00
	);

	SDL_SetColorKey(pSprite, true, 0xFF00FF);
	SDL_FillRect(pSprite, NULL, 0xFF00FF);

	int i, j;
	SDL_Rect rect = {0, 0, s_iSprWidth, s_iSprHeight};
	for(j=0; j<iHeight; j++) {
		for(i=0; i<iWidth; i++) {
			rect.x = s_iSprWidth*i, rect.y = s_iSprHeight*j;
			SDL_BlitSurface(s_pTiles[pIndices[iWidth*j+i]], NULL, pSprite, &rect);
		}
	}

	return pSprite;
}

//-----------------------------------------------------------------------------
SDL_Surface* AdSpriteManager::BuildSprite(SDL_Color sdlColor, const char* pString, ...) {
	if(pString == NULL) return NULL;

	int iLength = 0;
	char* pFmtString = NULL;

	va_list argp;
	va_start(argp, pString);

	const char* c;
	for(c=pString; *c!='\0'; ++c) {
		if(*c != '%') {
			pFmtString = (char*) realloc(pFmtString, ++iLength*sizeof(char));
			pFmtString[iLength-1] = *c;
			continue;
		}

		switch(*++c) {
			case 'd': {
				char temp[NAME_LENGTH];
				int i = va_arg(argp, int);
				sprintf(temp, "%d", i);

				pFmtString = (char*) realloc(
					pFmtString, (iLength+strlen(temp))*sizeof(char)
				);

				memcpy(pFmtString+iLength, temp, strlen(temp));
				iLength += strlen(temp);
			} break;

			case 's': {
				char* s = va_arg(argp, char*);

				pFmtString = (char*) realloc(
					pFmtString, (iLength+strlen(s))*sizeof(char)
				);

				memcpy(pFmtString+iLength, s, strlen(s));
				iLength += strlen(s);
			} break;

			default: {
				pFmtString = (char*) realloc(pFmtString, ++iLength*sizeof(char));
				pFmtString[iLength-1] = *c;
			} break;
		}
	}

	pFmtString = (char*) realloc(pFmtString, ++iLength*sizeof(char));
	pFmtString[iLength-1] = '\0';

	va_end(argp);

	SDL_Surface* pSprite;
	pSprite = TTF_RenderText_Solid(s_pFont, pFmtString, sdlColor);

	return pSprite;
}
