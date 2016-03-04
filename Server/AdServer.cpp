/*
*/

#include "AdServer.h"

//-----------------------------------------------------------------------------
AdServer::AdServer(void) {
	m_iNextInd = 0;
	m_pServerSocket = NULL;
	m_pSocketSet = NULL;

	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, NULL, 8098) == -1) {
		fprintf(stderr, "ER: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(-1);
	}

	m_pServerSocket = SDLNet_TCP_Open(&ip);
	if (m_pServerSocket == NULL) {
		fprintf(stderr, "ER: SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(-1);
	}

	m_pSocketSet = SDLNet_AllocSocketSet(MAX_SOCKETS + 1);
	if (m_pSocketSet == NULL) {
		fprintf(stderr, "ER: SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		exit(-1);
	}

	if (SDLNet_TCP_AddSocket(m_pSocketSet, m_pServerSocket) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
AdServer::~AdServer(void) {
	if (m_pSocketSet) {
		if (SDLNet_TCP_DelSocket(m_pSocketSet, m_pServerSocket) == -1) {
			fprintf(stderr, "ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
			exit(-1);
		}
	}
	
	if(m_pServerSocket) SDLNet_TCP_Close(m_pServerSocket);

	int i;
	for (i = 0; i<MAX_SOCKETS; ++i) {
		if (!m_pClients[i].IsActive()) continue;
		CloseSocket(i);
	}

	if(m_pSocketSet) SDLNet_FreeSocketSet(m_pSocketSet);
}

//-----------------------------------------------------------------------------
void AdServer::CloseSocket(int iIndex) {
	if (!m_pClients[iIndex].IsActive()) {
		fprintf(stderr, "ER: Attempted to delete a NULL socket.\n");
		return;
	}

	if (m_pClients[iIndex].RemoveFromSocketSet(m_pSocketSet) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
		exit(-1);
	}

	m_pClients[iIndex].Close();

	uint16_t send_offset = 0;
	uint8_t send_data[MAX_PACKET];

	Uint16 flag = FLAG_PLAYER_DISCONNECT;

	memcpy(send_data + send_offset, &flag, sizeof(Uint16));
	send_offset += sizeof(Uint16);
	memcpy(send_data + send_offset, &iIndex, sizeof(uint32_t));
	send_offset += sizeof(uint32_t);

	int ind2;
	for (ind2 = 0; ind2<MAX_SOCKETS; ++ind2) {
		if (ind2 == iIndex) continue;
		if (!m_pClients[ind2].IsActive()) continue;

		m_pClients[iIndex].SendData(send_data, send_offset);
	}
}

//-----------------------------------------------------------------------------
bool AdServer::AcceptSocket(int iIndex) {
	if (m_pClients[iIndex].IsActive()) {
		fprintf(stderr, "ER: Overriding socket at index %d.\n", iIndex);
		CloseSocket(iIndex);
	}

	if (m_pClients[iIndex].Init(m_pServerSocket) == false)
		return false;

	if (m_pClients[iIndex].AddToSocketSet(m_pSocketSet) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
		exit(-1);
	}

	return true;
}

//-----------------------------------------------------------------------------
Uint8* AdServer::RecvData(int iIndex, Uint16& sLength) {
	if (!m_pClients[iIndex].IsActive()) return NULL;

	uint8_t temp_data[MAX_PACKET];
	int num_recv = m_pClients[iIndex].RecvData(temp_data, MAX_PACKET);

	if (num_recv <= 0) {
		CloseSocket(iIndex);

		const char* err = SDLNet_GetError();
		if (strlen(err) == 0) {

			uint16_t send_offset = 0;
			uint8_t send_data[MAX_PACKET];

			Uint16 flag = FLAG_PLAYER_DISCONNECT;

			memcpy(send_data + send_offset, &flag, sizeof(Uint16));
			send_offset += sizeof(Uint16);
			memcpy(send_data + send_offset, &iIndex, sizeof(uint32_t));
			send_offset += sizeof(uint32_t);

			int ind;
			for (ind = 0; ind<MAX_SOCKETS; ++ind) {
				if (ind == iIndex) continue;
				if (!m_pClients[ind].IsActive()) continue;

				m_pClients[ind].SendData(send_data, send_offset);
			}

			printf("DB: client disconnected\n");
		}
		else {
			fprintf(stderr, "ER: SDLNet_TCP_Recv: %s\n", err);
		}

		return NULL;
	}
	else {
		sLength = num_recv;

		uint8_t* data = (uint8_t*)malloc(num_recv*sizeof(uint8_t));
		memcpy(data, temp_data, num_recv);

		return data;
	}
}

//-----------------------------------------------------------------------------
int AdServer::CheckSockets(void) {
	return SDLNet_CheckSockets(m_pSocketSet, 50);
}

//-----------------------------------------------------------------------------
void AdServer::UpdateClients(void) {
	// NOTE: none of the sockets are ready
	int ind;
	for (ind = 0; ind<MAX_SOCKETS; ++ind) {
		if (!m_pClients[ind].IsActive()) continue;

		//
		uint16_t send_offset = 0;
		uint8_t send_data[MAX_PACKET];

		if (m_pClients[ind].questing) {
			if ((SDL_GetTicks() - m_pClients[ind].timer_wood)>WOOD_WAIT_TIME) {
				//
				m_pClients[ind].questing = 0;
				m_pClients[ind].amt_wood += rand() % 3 + 3;//4;

				send_offset = 0;
				memset(send_data, 0x00, MAX_PACKET);

				Uint16 flag = FLAG_WOOD_QUEST;

				memcpy(send_data + send_offset, &flag, sizeof(Uint16));
				send_offset += sizeof(Uint16);

				m_pClients[ind].SendData(send_data, send_offset);

				//
				send_offset = 0;
				memset(send_data, 0x00, MAX_PACKET);

				flag = FLAG_WOOD_UPDATE;

				memcpy(send_data + send_offset, &flag, sizeof(Uint16));
				send_offset += sizeof(Uint16);
				memcpy(send_data + send_offset, &m_pClients[ind].amt_wood, sizeof(uint8_t));
				send_offset += sizeof(uint8_t);

				m_pClients[ind].SendData(send_data, send_offset);
			}

			//
			uint32_t time_left;
			if (m_pClients[ind].questing) {
				time_left = WOOD_WAIT_TIME - (SDL_GetTicks() - m_pClients[ind].timer_wood);
			}
			else time_left = 0;

			send_offset = 0;
			memset(send_data, 0x00, MAX_PACKET);

			Uint16 flag = FLAG_WOOD_GETTIME;

			memcpy(send_data + send_offset, &flag, sizeof(Uint16));
			send_offset += sizeof(Uint16);
			memcpy(send_data + send_offset, &time_left, sizeof(uint32_t));
			send_offset += sizeof(uint32_t);

			m_pClients[ind].SendData(send_data, send_offset);
		}
		//

		//
		send_offset = 0;
		memset(send_data, 0x00, MAX_PACKET);

		Uint16 flag = FLAG_PLAYER_POS;

		memcpy(send_data + send_offset, &flag, sizeof(Uint16));
		send_offset += sizeof(Uint16);
		memcpy(send_data + send_offset, &ind, sizeof(uint32_t));
		send_offset += sizeof(uint32_t);
		memcpy(send_data + send_offset, &m_pClients[ind].i, sizeof(uint8_t));
		send_offset += sizeof(uint8_t);
		memcpy(send_data + send_offset, &m_pClients[ind].j, sizeof(uint8_t));
		send_offset += sizeof(uint8_t);

		int ind2;
		for (ind2 = 0; ind2<MAX_SOCKETS; ++ind2) {
			if (ind2 == ind) continue;
			if (!m_pClients[ind2].IsActive()) continue;

			m_pClients[ind2].SendData(send_data, send_offset);
		}
		//
	}
}

//-----------------------------------------------------------------------------
void AdServer::ProcPackets(int& iNumReady) {
	if (SDLNet_SocketReady(m_pServerSocket)) {
		if (CheckForNewConnection()) {
			printf("DB: new connection (next_ind = %d)\n", m_iNextInd);
		}

		iNumReady--;
	}

	int ind;
	for (ind = 0; (ind<MAX_SOCKETS) && iNumReady; ++ind) {
		if (!m_pClients[ind].IsActive()) continue;
		if (!m_pClients[ind].IsSocketReady()) continue;

		uint8_t* data;
		uint16_t length;

		data = RecvData(ind, length);
		if (data == NULL) {
			iNumReady--;
			continue;
		}

		int num_processed = 0;

		uint16_t offset = 0;
		while (offset < length) {
			num_processed++;
			m_pClients[ind].ProcData(data, offset);
		}

		//printf("num_processed from ID: %d was %d\n", ind, num_processed);
		//fflush(stdout);

		free(data);
		iNumReady--;
	}
}

//-----------------------------------------------------------------------------
bool AdServer::CheckForNewConnection(void) {
	if (!AcceptSocket(m_iNextInd)) {
		return false;
	}

	uint16_t send_offset = 0;
	uint16_t send_offset2 = 0;
	uint8_t send_data[MAX_PACKET];
	uint8_t send_data2[MAX_PACKET];

	Uint16 flag = FLAG_PLAYER_CONNECT;

	memcpy(send_data + send_offset, &flag, sizeof(Uint16));
	send_offset += sizeof(Uint16);
	memcpy(send_data + send_offset, &m_iNextInd, sizeof(uint32_t));
	send_offset += sizeof(uint32_t);
	memcpy(send_data + send_offset, &m_pClients[m_iNextInd].i, sizeof(uint8_t));
	send_offset += sizeof(uint8_t);
	memcpy(send_data + send_offset, &m_pClients[m_iNextInd].j, sizeof(uint8_t));
	send_offset += sizeof(uint8_t);

	int ind;
	for (ind = 0; ind<MAX_SOCKETS; ++ind) {
		if (ind == m_iNextInd) continue;
		if (!m_pClients[ind].IsActive()) continue;

		m_pClients[ind].SendData(send_data, send_offset);

		send_offset2 = 0;

		memcpy(send_data2 + send_offset2, &flag, sizeof(Uint16));
		send_offset2 += sizeof(Uint16);
		memcpy(send_data2 + send_offset2, &ind, sizeof(uint32_t));
		send_offset2 += sizeof(uint32_t);
		memcpy(send_data2 + send_offset2, &m_pClients[ind].i, sizeof(uint8_t));
		send_offset2 += sizeof(uint8_t);
		memcpy(send_data2 + send_offset2, &m_pClients[ind].j, sizeof(uint8_t));
		send_offset2 += sizeof(uint8_t);

		m_pClients[m_iNextInd].SendData(send_data2, send_offset2);
	}

	// NOTE: get a new index
	int chk_count;
	for (chk_count = 0; chk_count<MAX_SOCKETS; ++chk_count) {
		if (!m_pClients[(m_iNextInd + chk_count) % MAX_SOCKETS].IsActive()) break;
	}

	m_iNextInd = (m_iNextInd + chk_count) % MAX_SOCKETS;

	return true;
}
