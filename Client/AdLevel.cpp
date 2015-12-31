/*
*/

#include "AdLevel.h"
#include "AdScreen.h"
#include "AdSpriteManager.h"

//-----------------------------------------------------------------------------
AdLevel::AdLevel(void) {
	memset(m_pFileName, 0x00, NAME_LENGTH);
	memset(m_pClients, 0x00, MAX_SOCKETS*sizeof(AdEntity*));

	m_pLayers = NULL;
	m_pPlayer = NULL;

	m_iMouseX     = 0;
	m_iMouseY     = 0;
	m_bMouseLeft  = false;
	m_bMouseRight = false;
}

//-----------------------------------------------------------------------------
AdLevel::~AdLevel(void) {
	Unload();
}

//-----------------------------------------------------------------------------
void AdLevel::Update(SDL_Event* sdlEvent) {
	// NOTE: automatically reload the level when 'r' is pressed
	static bool bReload = false;
	static bool bChkReload = false;

	switch(sdlEvent->type) {
		case SDL_KEYDOWN: {
			switch(sdlEvent->key.keysym.sym) {
				case SDLK_r: bReload = true; break;
			}
		} break;

		case SDL_KEYUP: {
			switch(sdlEvent->key.keysym.sym) {
				case SDLK_r: bReload = false; break;
			}
		} break;
	}

	if(bReload && !bChkReload) {
		char pFileName[NAME_LENGTH];
		strcpy(pFileName, m_pFileName);

		Load(pFileName);

	} else if(!bReload) bChkReload = false;

	if(m_pPlayer != NULL) {
		// NOTE: update the player's input
		if(!m_pPlayer->m_bUp)    m_pPlayer->m_bUpCheck    = false;
		if(!m_pPlayer->m_bDown)  m_pPlayer->m_bDownCheck  = false;
		if(!m_pPlayer->m_bLeft)  m_pPlayer->m_bLeftCheck  = false;
		if(!m_pPlayer->m_bRight) m_pPlayer->m_bRightCheck = false;
	}

	switch(sdlEvent->type) {
		case SDL_MOUSEMOTION: {
			m_iMouseX = sdlEvent->motion.x/AdBase::GetScale();
			m_iMouseY = sdlEvent->motion.y/AdBase::GetScale();
		} break;

		case SDL_MOUSEBUTTONDOWN: {
			switch(sdlEvent->button.button) {
				case SDL_BUTTON_LEFT:  m_bMouseLeft  = true; break;
				case SDL_BUTTON_RIGHT: m_bMouseRight = true; break;
			}
		} break;

		case SDL_MOUSEBUTTONUP: {
			switch(sdlEvent->button.button) {
				case SDL_BUTTON_LEFT:  m_bMouseLeft  = false; break;
				case SDL_BUTTON_RIGHT: m_bMouseRight = false; break;
			}
		} break;

		case SDL_KEYDOWN: {
			if(m_pPlayer == NULL) break;
			switch(sdlEvent->key.keysym.sym) {
				case SDLK_UP:    m_pPlayer->m_bUp    = true; break;
				case SDLK_DOWN:  m_pPlayer->m_bDown  = true; break;
				case SDLK_LEFT:  m_pPlayer->m_bLeft  = true; break;
				case SDLK_RIGHT: m_pPlayer->m_bRight = true; break;
			}
		} break;

		case SDL_KEYUP: {
			if(m_pPlayer == NULL) break;
			switch(sdlEvent->key.keysym.sym) {
				case SDLK_UP:    m_pPlayer->m_bUp    = false; break;
				case SDLK_DOWN:  m_pPlayer->m_bDown  = false; break;
				case SDLK_LEFT:  m_pPlayer->m_bLeft  = false; break;
				case SDLK_RIGHT: m_pPlayer->m_bRight = false; break;
			}
		} break;
	}

	for(int e=0; e<m_objMap.nEntities(); ++e) {
		AdEntity* pEnt = m_objMap.GetEntity(e);
		pEnt->Update(this);

		if(!pEnt->IsTriggered()) continue;

		// NOTE: check for transition entities
		if(strlen(pEnt->m_pSendTo) != 0) {
			if(!strcmp(pEnt->GetType(), "LVL-UP")) {
				AdEntity* pPlayer = m_pPlayer;
				int sendto_offset = pEnt->m_iSendToOffset;

				Load(pEnt->m_pSendTo);

				pPlayer->m_recTrigger.y += AdBase::GetHeight()-8*4 + sendto_offset;
				SetPlayer(pPlayer);

				return;
			} else if(!strcmp(pEnt->GetType(), "LVL-DOWN")) {
				AdEntity* pPlayer = m_pPlayer;
				int sendto_offset = pEnt->m_iSendToOffset;

				Load(pEnt->m_pSendTo);

				pPlayer->m_recTrigger.y -= AdBase::GetHeight()-8*4 + sendto_offset;
				SetPlayer(pPlayer);

				return;
			} else if(!strcmp(pEnt->GetType(), "LVL-LEFT")) {
				AdEntity* pPlayer = m_pPlayer;
				int sendto_offset = pEnt->m_iSendToOffset;

				Load(pEnt->m_pSendTo);

				pPlayer->m_recTrigger.x += AdBase::GetWidth()-8*4 + sendto_offset;
				SetPlayer(pPlayer);

				return;
			} else if(!strcmp(pEnt->GetType(), "LVL-RIGHT")) {
				AdEntity* pPlayer = m_pPlayer;
				int sendto_offset = pEnt->m_iSendToOffset;

				Load(pEnt->m_pSendTo);

				pPlayer->m_recTrigger.x -= AdBase::GetWidth()-8*4 + sendto_offset;
				SetPlayer(pPlayer);

				return;
			}
		}
	}

	for(int ind=0; ind<MAX_SOCKETS; ++ind) {
		if(m_pClients[ind] == NULL) continue;
		m_pClients[ind]->Update(this);
	}

	if(m_pPlayer) m_pPlayer->Update(this);
}

//-----------------------------------------------------------------------------
void AdLevel::Render(void) {
	for(int j=0; j<m_objMap.nLayers(); ++j) {
		if(j == COLLISION_LAYER) continue;

		SDL_Point pntLvl = {0, 0};
		AdScreen::DrawSprite(pntLvl, m_pLayers[j]);

		if(j == PLAYER_DRAW_LAYER) {
			// NOTE: render entities
			for(int e=0; e<m_objMap.nEntities(); ++e) {
				m_objMap.GetEntity(e)->Render(this);
			}

			for(int ind=0; ind<MAX_SOCKETS; ++ind) {
				if(m_pClients[ind] == NULL) continue;
				m_pClients[ind]->Render(this);
			}

			if(m_pPlayer) m_pPlayer->Render(this);
		}
	}
}

//-----------------------------------------------------------------------------
void AdLevel::Load(const char* pFileName) {
	Unload();

	strcpy(m_pFileName, pFileName);

	m_objMap.Load(m_pFileName);

	// NOTE: get the moveable entity to follow with camera
	for(int e=0; e<m_objMap.nEntities(); ++e) {
		AdEntity* pEnt = m_objMap.GetEntity(e);
	}

	// NOTE: create an SDL_Surface for each map layer
	m_pLayers = (SDL_Surface**) malloc(m_objMap.nLayers()*sizeof(SDL_Surface*));

	for(int j=0; j<m_objMap.nLayers(); ++j) {
		if(m_objMap.GetLayer(j)) {
			m_pLayers[j] = AdSpriteManager::BuildSprite(
				m_objMap.Width(), m_objMap.Height(), m_objMap.GetLayer(j)
			);
		} else {
			m_pLayers[j] = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
void AdLevel::Unload(void) {
	m_pPlayer = NULL;

	for(int j=0; j<m_objMap.nLayers(); ++j) {
		if(m_pLayers && m_pLayers[j]) {
			SDL_FreeSurface(m_pLayers[j]);
			m_pLayers[j] = NULL;
		}
	}

	if(m_pLayers) {
		free(m_pLayers);
		m_pLayers = NULL;
	}

	for(int ind=0; ind<MAX_SOCKETS; ++ind) {
		if(m_pClients[ind] == NULL) continue;
		delete m_pClients[ind];
	}

	memset(m_pFileName, 0x00, NAME_LENGTH);
	memset(m_pClients, 0x00, MAX_SOCKETS*sizeof(AdEntity*));
}
