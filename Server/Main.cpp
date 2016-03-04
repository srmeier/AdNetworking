/*
*/

//-----------------------------------------------------------------------------
#include "AdDefine.h"
#include "AdSocket.h"
#include "AdClient.h"
#include "AdServer.h"

//-----------------------------------------------------------------------------
int running = 1;

//-----------------------------------------------------------------------------
int SDL_main(int argc, char** argv) {
	srand((unsigned int) time(NULL));

	if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
		fprintf(stderr, "ER: SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	if(SDLNet_Init() == -1) {
		fprintf(stderr, "ER: SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}

	AdServer server;

	while(running) {
		int num_rdy = server.CheckSockets();

		if(num_rdy <= 0) {
			server.UpdateClients();
		} else {
			server.ProcPackets(num_rdy);

			if (num_rdy > 0)
				printf("WR: still have %d sockets let to process!\n", num_rdy);
		}
	}

	SDLNet_Quit();
	SDL_Quit();

	return 0;
}
