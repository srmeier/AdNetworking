/*
*/

//-----------------------------------------------------------------------------
#include "AdDefine.h"
#include "AdSocket.h"

//-----------------------------------------------------------------------------
typedef struct {
	int in_use;
	int questing;
	uint8_t i, j;
	uint8_t direc;
	uint8_t amt_wood;
	uint32_t timer_wood;
} Client;

//-----------------------------------------------------------------------------
int running = 1;
int next_ind = 0;
TCPsocket server_socket;
Client clients[MAX_SOCKETS];
SDLNet_SocketSet socket_set;
TCPsocket sockets[MAX_SOCKETS];

//-----------------------------------------------------------------------------
void SendData(int index, uint8_t* data, uint16_t length, uint16_t flag);

//-----------------------------------------------------------------------------
void CloseSocket(int index) {
	if(sockets[index] == NULL) {
		fprintf(stderr, "ER: Attempted to delete a NULL socket.\n");
		return;
	}

	if(SDLNet_TCP_DelSocket(socket_set, sockets[index]) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
		exit(-1);
	}

	memset(&clients[index], 0x00, sizeof(Client));
	SDLNet_TCP_Close(sockets[index]);
	sockets[index] = NULL;

	//
	uint16_t send_offset = 0;
	uint8_t send_data[MAX_PACKET];

	memcpy(send_data+send_offset, &index, sizeof(uint32_t));
	send_offset += sizeof(uint32_t);

	int ind2;
	for(ind2=0; ind2<MAX_SOCKETS; ++ind2) {
		if(ind2 == index) continue;
		if(!clients[ind2].in_use) continue;
		SendData(ind2, send_data, send_offset, FLAG_PLAYER_DISCONNECT);
	}
	//
}

//-----------------------------------------------------------------------------
int AcceptSocket(int index) {
	if(sockets[index]) {
		fprintf(stderr, "ER: Overriding socket at index %d.\n", index);
		CloseSocket(index);
	}

	sockets[index] = SDLNet_TCP_Accept(server_socket);
	if(sockets[index] == NULL) return 0;

	clients[index].in_use = 1;
	if(SDLNet_TCP_AddSocket(socket_set, sockets[index]) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
		exit(-1);
	}

	return 1;
}

//-----------------------------------------------------------------------------
void SendData(int index, uint8_t* data, uint16_t length, uint16_t flag) {
	if(sockets[index] == NULL) return;

	int offset = 0;
	uint8_t temp_data[MAX_PACKET];

	memcpy(temp_data+offset, &flag, sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(temp_data+offset, data, length);
	offset += length;

	int num_sent = SDLNet_TCP_Send(sockets[index], temp_data, offset);
	if(num_sent < offset) {
		fprintf(stderr, "ER: SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		CloseSocket(index);
	}
}

//-----------------------------------------------------------------------------
uint8_t* RecvData(int index, uint16_t* length) {
	if(sockets[index] == NULL) return NULL;

	uint8_t temp_data[MAX_PACKET];
	int num_recv = SDLNet_TCP_Recv(sockets[index], temp_data, MAX_PACKET);

	if(num_recv <= 0) {
		CloseSocket(index);

		const char* err = SDLNet_GetError();
		if(strlen(err) == 0) {
			//
			uint16_t send_offset = 0;
			uint8_t send_data[MAX_PACKET];

			memcpy(send_data+send_offset, &index, sizeof(uint32_t));
			send_offset += sizeof(uint32_t);

			int ind2;
			for(ind2=0; ind2<MAX_SOCKETS; ++ind2) {
				if(ind2 == index) continue;
				if(!clients[ind2].in_use) continue;
				SendData(ind2, send_data, send_offset, FLAG_PLAYER_DISCONNECT);
			}
			//

			printf("DB: client disconnected\n");
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
void ProcessData(int index, uint8_t* data, uint16_t* offset) {
	if(data == NULL) return;
	if(sockets[index] == NULL) return;

	uint16_t flag = *(uint16_t*) &data[*offset];
	*offset += sizeof(uint16_t);

	switch(flag) {
		case FLAG_WOOD_UPDATE: {
			uint16_t send_offset = 0;
			uint8_t send_data[MAX_PACKET];

			memcpy(send_data+send_offset, &clients[index].amt_wood, sizeof(uint8_t));
			send_offset += sizeof(uint8_t);

			SendData(index, send_data, send_offset, FLAG_WOOD_UPDATE);
		} break;

		case FLAG_WOOD_GETTIME: {
			uint16_t send_offset = 0;
			uint8_t send_data[MAX_PACKET];

			uint32_t time_left;
			if(clients[index].questing) {
				time_left = WOOD_WAIT_TIME-(SDL_GetTicks()-clients[index].timer_wood);
			} else time_left = 0;

			memcpy(send_data+send_offset, &time_left, sizeof(uint32_t));
			send_offset += sizeof(uint32_t);

			SendData(index, send_data, send_offset, FLAG_WOOD_GETTIME);
		} break;

		case FLAG_WOOD_QUEST: {
			if(!clients[index].questing) {
				clients[index].questing = 1;
				clients[index].timer_wood = SDL_GetTicks();
			}
		} break;

		case FLAG_PLAYER_POS: {
			clients[index].i = *(uint8_t*) &data[*offset];
			*offset += sizeof(uint8_t);
			clients[index].j = *(uint8_t*) &data[*offset];
			*offset += sizeof(uint8_t);
			clients[index].direc = *(uint8_t*) &data[*offset];
			*offset += sizeof(uint8_t);
		} break;

		case FLAG_QUIT: {
			//running = 0;
			//printf("DB: shutdown by client id: %d\n", index);
		} break;
	}
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
	srand((unsigned int) time(NULL));

	if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
		fprintf(stderr, "ER: SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	if(SDLNet_Init() == -1) {
		fprintf(stderr, "ER: SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}

	IPaddress ip;
	if(SDLNet_ResolveHost(&ip, NULL, 8098) == -1) {
		fprintf(stderr, "ER: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(-1);
	}

	server_socket = SDLNet_TCP_Open(&ip);
	if(server_socket == NULL) {
		fprintf(stderr, "ER: SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(-1);
	}

	socket_set = SDLNet_AllocSocketSet(MAX_SOCKETS+1);
	if(socket_set == NULL) {
		fprintf(stderr, "ER: SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		exit(-1);
	}

	if(SDLNet_TCP_AddSocket(socket_set, server_socket) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
		exit(-1);
	}

	while(running) {
		int num_rdy = SDLNet_CheckSockets(socket_set, 50);

		if(num_rdy <= 0) {
			// NOTE: none of the sockets are ready
			int ind;
			for(ind=0; ind<MAX_SOCKETS; ++ind) {
				if(!clients[ind].in_use || sockets[ind]==NULL) continue;

				//
				uint16_t send_offset = 0;
				uint8_t send_data[MAX_PACKET];

				if(clients[ind].questing) {
					if((SDL_GetTicks()-clients[ind].timer_wood)>WOOD_WAIT_TIME) {
						clients[ind].questing = 0;
						clients[ind].amt_wood += rand()%3+3;//4;
						SendData(ind, NULL, 0, FLAG_WOOD_QUEST);

						//
						uint16_t send_offset = 0;
						uint8_t send_data[MAX_PACKET];

						memcpy(send_data+send_offset, &clients[ind].amt_wood, sizeof(uint8_t));
						send_offset += sizeof(uint8_t);

						SendData(ind, send_data, send_offset, FLAG_WOOD_UPDATE);
						//
					}

					uint32_t time_left;
					if(clients[ind].questing) {
						time_left = WOOD_WAIT_TIME-(SDL_GetTicks()-clients[ind].timer_wood);
					} else time_left = 0;

					memcpy(send_data+send_offset, &time_left, sizeof(uint32_t));
					send_offset += sizeof(uint32_t);

					SendData(ind, send_data, send_offset, FLAG_WOOD_GETTIME);
				}
				//

				//
				send_offset = 0;
				memcpy(send_data+send_offset, &ind, sizeof(uint32_t));
				send_offset += sizeof(uint32_t);
				memcpy(send_data+send_offset, &clients[ind].i, sizeof(uint8_t));
				send_offset += sizeof(uint8_t);
				memcpy(send_data+send_offset, &clients[ind].j, sizeof(uint8_t));
				send_offset += sizeof(uint8_t);

				int ind2;
				for(ind2=0; ind2<MAX_SOCKETS; ++ind2) {
					if(ind2 == ind) continue;
					if(!clients[ind2].in_use) continue;
					SendData(ind2, send_data, send_offset, FLAG_PLAYER_POS);
				}
				//
			}
		} else {
			if(SDLNet_SocketReady(server_socket)) {
				int got_socket = AcceptSocket(next_ind);
				if(!got_socket) {
					num_rdy--;
					continue;
				}

				//
				uint16_t send_offset = 0;
				uint16_t send_offset2 = 0;
				uint8_t send_data[MAX_PACKET];
				uint8_t send_data2[MAX_PACKET];

				memcpy(send_data+send_offset, &next_ind, sizeof(uint32_t));
				send_offset += sizeof(uint32_t);
				memcpy(send_data+send_offset, &clients[next_ind].i, sizeof(uint8_t));
				send_offset += sizeof(uint8_t);
				memcpy(send_data+send_offset, &clients[next_ind].j, sizeof(uint8_t));
				send_offset += sizeof(uint8_t);

				int ind2;
				for(ind2=0; ind2<MAX_SOCKETS; ++ind2) {
					if(ind2 == next_ind) continue;
					if(!clients[ind2].in_use) continue;
					SendData(ind2, send_data, send_offset, FLAG_PLAYER_CONNECT);

					send_offset2 = 0;
					memcpy(send_data2+send_offset2, &ind2, sizeof(uint32_t));
					send_offset2 += sizeof(uint32_t);
					memcpy(send_data2+send_offset2, &clients[ind2].i, sizeof(uint8_t));
					send_offset2 += sizeof(uint8_t);
					memcpy(send_data2+send_offset2, &clients[ind2].j, sizeof(uint8_t));
					send_offset2 += sizeof(uint8_t);
					SendData(next_ind, send_data2, send_offset2, FLAG_PLAYER_CONNECT);
				}
				//

				// NOTE: get a new index
				int chk_count;
				for(chk_count=0; chk_count<MAX_SOCKETS; ++chk_count) {
					if(sockets[(next_ind+chk_count)%MAX_SOCKETS] == NULL) break;
				}

				next_ind = (next_ind+chk_count)%MAX_SOCKETS;
				printf("DB: new connection (next_ind = %d)\n", next_ind);

				num_rdy--;
			}

			int ind;
			for(ind=0; (ind<MAX_SOCKETS) && num_rdy; ++ind) {
				if(!clients[ind].in_use || sockets[ind]==NULL) continue;
				if(!SDLNet_SocketReady(sockets[ind])) continue;

				uint8_t* data;
				uint16_t length;
				
				data = RecvData(ind, &length);
				if(data == NULL) {
					num_rdy--;
					continue;
				}

				int num_processed = 0;

				uint16_t offset = 0;
				while(offset < length) {
					num_processed++;
					ProcessData(ind, data, &offset);
				}

				//printf("num_processed from ID: %d was %d\n", ind, num_processed);
				fflush(stdout);

				free(data);
				num_rdy--;
			}
		}
	}

	if(SDLNet_TCP_DelSocket(socket_set, server_socket) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
		exit(-1);
	} SDLNet_TCP_Close(server_socket);

	int i;
	for(i=0; i<MAX_SOCKETS; ++i) {
		if(sockets[i] == NULL) continue;
		CloseSocket(i);
	}

	SDLNet_FreeSocketSet(socket_set);
	SDLNet_Quit();
	SDL_Quit();

	return 0;
}
