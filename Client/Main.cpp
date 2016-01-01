/*
*/

#include "AdBase.h"
#include "AdLevel.h"
#include "AdScreen.h"
#include "AdEntity.h"
#include "AdSpriteManager.h"

//-----------------------------------------------------------------------------
TCPsocket socket;
SDLNet_SocketSet socket_set;

//-----------------------------------------------------------------------------
int questing;
uint8_t amt_wood;
uint32_t timer_wood;

AdLevel testLvl;

//-----------------------------------------------------------------------------
void CloseSocket(void) {
	if(socket == NULL) return;

	if(SDLNet_TCP_DelSocket(socket_set, socket) == -1) {
		fprintf(stderr, "%s\n", SDLNet_GetError());
		system("pause");
		exit(-1);
	}

	SDLNet_FreeSocketSet(socket_set);
	SDLNet_TCP_Close(socket);
	socket = NULL;
}

//-----------------------------------------------------------------------------
void SendData(uint8_t* data, uint16_t length, uint16_t flag) {
	if(socket == NULL) return;

	int offset = 0;
	uint8_t temp_data[MAX_PACKET];

	memcpy(temp_data+offset, &flag, sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(temp_data+offset, data, length);
	offset += length;

	int num_sent = SDLNet_TCP_Send(socket, temp_data, offset);
	if(num_sent < offset) {
		fprintf(stderr, "ER: SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		CloseSocket();
	}
}

//-----------------------------------------------------------------------------
uint8_t* RecvData(uint16_t* length) {
	if(socket == NULL) return NULL;

	uint8_t temp_data[MAX_PACKET];
	int num_recv = SDLNet_TCP_Recv(socket, temp_data, MAX_PACKET);

	if(num_recv <= 0) {
		CloseSocket();

		const char* err = SDLNet_GetError();
		if(strlen(err) == 0) {
			printf("DB: server shutdown\n");

			for(int ind=0; ind<MAX_SOCKETS; ++ind) {
				if(testLvl.m_pClients[ind] == NULL) continue;
				
				delete testLvl.m_pClients[ind];
				testLvl.m_pClients[ind] = NULL;
			}

		} else {
			fprintf(stderr, "ER: SDLNet_TCP_Recv: %s\n", err);
		}

		return NULL;
	} else {
		*length = num_recv;

		uint8_t* data = (uint8_t*) malloc(num_recv*sizeof(uint8_t));
		memcpy(data, temp_data, num_recv);

		return data;
	}
}

//-----------------------------------------------------------------------------
void ProcessData(uint8_t* data, uint16_t* offset) {
	if(data == NULL) return;
	if(socket == NULL) return;

	uint16_t flag = *(uint16_t*) &data[*offset];
	*offset += sizeof(uint16_t);

	switch(flag) {
		case FLAG_WOOD_UPDATE: {
			amt_wood = *(uint8_t*) &data[*offset];
			*offset += sizeof(uint8_t);
		} break;

		case FLAG_WOOD_GETTIME: {
			timer_wood = *(uint32_t*) &data[*offset];
			*offset += sizeof(uint32_t);
		} break;

		case FLAG_WOOD_QUEST: {
			questing = 0; // NOTE: quest completed
		} break;

		case FLAG_PLAYER_CONNECT: {
			int ind = *(uint32_t*) &data[*offset];
			*offset += sizeof(uint32_t);
			int m_iGI = *(uint8_t*) &data[*offset];
			*offset += sizeof(uint8_t);
			int m_iGJ = *(uint8_t*) &data[*offset];
			*offset += sizeof(uint8_t);

			testLvl.m_pClients[ind] = new AdEntity(8*m_iGI, 8*m_iGJ, "player");
		} break;

		case FLAG_PLAYER_DISCONNECT: {
			int ind = *(uint32_t*) &data[*offset];
			*offset += sizeof(uint32_t);

			delete testLvl.m_pClients[ind];
			testLvl.m_pClients[ind] = NULL;
		}

		case FLAG_PLAYER_POS: {
			int ind = *(uint32_t*) &data[*offset];
			*offset += sizeof(uint32_t);

			AdEntity* client = testLvl.m_pClients[ind];
			if(client == NULL) {
				printf("ER: missing client! (%d)\n", ind);
				int m_iGI = *(uint8_t*) &data[*offset];
				*offset += sizeof(uint8_t);
				int m_iGJ = *(uint8_t*) &data[*offset];
				*offset += sizeof(uint8_t);
			} else {
				client->m_iGI = *(uint8_t*) &data[*offset];
				*offset += sizeof(uint8_t);
				client->m_iGJ = *(uint8_t*) &data[*offset];
				*offset += sizeof(uint8_t);
			}
		} break;
	}
}

//-----------------------------------------------------------------------------
void InitNetwork(const char* pIP, int iPort) {
	IPaddress ip;
	if(SDLNet_ResolveHost(&ip, pIP, iPort) == -1) {
		fprintf(stderr, "%s\n", SDLNet_GetError());
		system("pause");
		exit(-1);
	}

	socket = SDLNet_TCP_Open(&ip);
	if(socket == NULL) {
		fprintf(stderr, "%s\n", SDLNet_GetError());
		system("pause");
		exit(-1);
	}

	socket_set = SDLNet_AllocSocketSet(1);
	if(socket_set == NULL) {
		fprintf(stderr, "%s\n", SDLNet_GetError());
		system("pause");
		exit(-1);
	}

	if(SDLNet_TCP_AddSocket(socket_set, socket) == -1) {
		fprintf(stderr, "%s\n", SDLNet_GetError());
		system("pause");
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
bool CheckSocket(void) {
	if(socket == NULL) return false;

	if(SDLNet_CheckSockets(socket_set, 0) == -1) {
		fprintf(stderr, "%s\n", SDLNet_GetError());
		system("pause");
		exit(-1);
	}

	return (SDLNet_SocketReady(socket) != 0);
}

//-----------------------------------------------------------------------------
int SDL_main(int argc, char* argv[]) {
	if(AdBase::Init(8*40, 8*30, 2) == false) {
		fprintf(stderr, "ERROR: Failed to initiate.\n");
		system("pause");
		return -1;
	}

	// TESTING
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
		fprintf(stderr, "ER: %s\n", Mix_GetError());
		system("pause");
		return -1;
	}

	Mix_Music* music;
	music = Mix_LoadMUS("data/sounds/Zero Cool 2.mp3");
	if(music == NULL) {
		fprintf(stderr, "ER: %s\n", Mix_GetError());
		system("pause");
		return -1;
	}

	if(Mix_PlayMusic(music, -1) == -1) {
		fprintf(stderr, "ER: %s\n", Mix_GetError());
		system("pause");
		return -1;
	}
	//

	//
	SDL_RWops* file = SDL_RWFromFile("data/sounds/qubodupImpactWood.ogg", "rb");
	if(file == NULL) {
		fprintf(stderr, "ER: %s\n", SDL_GetError());
		system("pause");
		return -1;
	}

	Mix_Chunk* sample;
	sample = Mix_LoadWAV_RW(file, 1);
	if(sample == NULL) {
		fprintf(stderr, "ER: %s\n", Mix_GetError());
		system("pause");
		return -1;
	}

	Mix_VolumeMusic(0);//MIX_MAX_VOLUME/4);
	Mix_VolumeChunk(sample, 0);//MIX_MAX_VOLUME/4);
	//

	// TESTING
	//InitNetwork("23.88.121.38", 8098);
	InitNetwork("127.0.0.1", 8098);

	AdEntity player(8, 8, "player");

	testLvl.Load("testing");
	testLvl.SetPlayer(&player);
	//

	SDL_Event sdlEvent = {};
	while(sdlEvent.type != SDL_QUIT) {
		SDL_PollEvent(&sdlEvent);

		AdScreen::Clear();

		// TESTING
		uint16_t send_offset = 0;
		uint8_t send_data[MAX_PACKET];

		if(CheckSocket()) {
			uint16_t length = 0;
			uint8_t* data = RecvData(&length);

			int num_processed = 0;

			uint16_t offset = 0;
			while(offset < length) {
				num_processed++;
				ProcessData(data, &offset);
			}

			//printf("DB: num_processed = %d\n", num_processed);

			free(data);
		}
		//

		//
		int iIold = player.m_iI;
		int iJold = player.m_iJ;

		testLvl.Update(&sdlEvent);

		for(int ind=0; ind<MAX_SOCKETS; ++ind) {
			if(testLvl.m_pClients[ind] == NULL) continue;
			testLvl.m_pClients[ind]->m_iFrame = testLvl.m_pClients[ind]->m_iForcedirec;
		}

		player.m_iFrame = player.m_iMovedirec;

		if(player.m_iI!=iIold || player.m_iJ!=iJold) {
			send_offset = 0;
			memcpy(send_data+send_offset, &player.m_iI, sizeof(uint8_t));
			send_offset += sizeof(uint8_t);
			memcpy(send_data+send_offset, &player.m_iJ, sizeof(uint8_t));
			send_offset += sizeof(uint8_t);
			memcpy(send_data+send_offset, &player.m_iMovedirec, sizeof(uint8_t));
			send_offset += sizeof(uint8_t);

			SendData(send_data, send_offset, FLAG_PLAYER_POS);
		}
		//

		// TESTING
		testLvl.Render();

		SDL_Color color1 = {0xFF, 0x00, 0x00, 0x00};
		SDL_Surface* pSurf = AdSpriteManager::BuildSprite(
			color1, "Wood: %d", amt_wood
		);

		SDL_Point pnt1 = {0, 0};
		AdScreen::DrawSprite(pnt1, pSurf);
		SDL_FreeSurface(pSurf);

		pSurf = AdSpriteManager::BuildSprite(
			color1, "Timer: %d", (int) ceil((double)timer_wood/1000.0f)
		);

		pnt1.y += 8;
		AdScreen::DrawSprite(pnt1, pSurf);
		SDL_FreeSurface(pSurf);
		//

		//
		if(timer_wood == 0) {
			SDL_Point pnt2 = {128, 8};
			SDL_Rect rec2 = {pnt2.x, pnt2.y, 5*8, 8};

			if(
				testLvl.m_iMouseX>=rec2.x && testLvl.m_iMouseX<=(rec2.x+rec2.w) &&
				testLvl.m_iMouseY>=rec2.y && testLvl.m_iMouseY<=(rec2.y+rec2.h)
			) {
				SDL_Color color2 = {0x00, 0xFF, 0x00, 0x00};
				pSurf = AdSpriteManager::BuildSprite(color2, "Quest");

				if(testLvl.m_bMouseLeft && !questing) {
					questing = 1;
					SendData(NULL, 0, FLAG_WOOD_QUEST);
				}
			} else {
				SDL_Color color2 = {0x00, 0x00, 0xFF, 0x00};
				pSurf = AdSpriteManager::BuildSprite(color2, "Quest");
			}
			
			AdScreen::DrawSprite(pnt2, pSurf);
			SDL_FreeSurface(pSurf);
		} else {
			if(Mix_Playing(2) == false) {
				if(Mix_PlayChannel(2, sample, 0) == -1) {
					fprintf(stderr, "ER: %s\n", Mix_GetError());
					system("pause");
					return -1;
				}
			}
		}
		//

		AdScreen::Present();
	}

	// TESTING
	Mix_CloseAudio();

	SendData(NULL, 0, FLAG_QUIT);
	CloseSocket();
	//

	AdBase::Quit();

	return 0;
}
