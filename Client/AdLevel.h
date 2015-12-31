#ifndef __ADLEVEL_H_
#define __ADLEVEL_H_

#include "AdBase.h"
#include "AdEntity.h"
#include "AdTiledManager.h"

//-----------------------------------------------------------------------------
class AdLevel {
public:
	AdTiledManager m_objMap;
	SDL_Surface**  m_pLayers;
	AdEntity*      m_pPlayer;
	AdEntity*      m_pClients[MAX_SOCKETS];
	char           m_pFileName[NAME_LENGTH];

public:
	int  m_iMouseX;
	int  m_iMouseY;
	bool m_bMouseLeft;
	bool m_bMouseRight;

public:
	AdEntity*       GetPlayer(void) {return m_pPlayer;}
	AdTiledManager* GetTiledMap(void) {return &m_objMap;}
	void            SetPlayer(AdEntity* pPlayer) {m_pPlayer = pPlayer;}
	void            AddEntity(AdEntity* pEnt) {m_objMap.AddEntity(pEnt);}

public:
	void Update(SDL_Event* sdlEvent);
	void Render(void);

public:
	void Load(const char* pName);
	void Unload(void);

public:
	AdLevel(void);
	~AdLevel(void);
};

#endif
