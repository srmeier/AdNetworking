/*
*/

#include "AdClient.h"

//-----------------------------------------------------------------------------
AdClient::AdClient(void):AdSocket() {
	Reset();
}

//-----------------------------------------------------------------------------
AdClient::~AdClient(void) {
	Reset();

	AdSocket::~AdSocket();
}

//-----------------------------------------------------------------------------
void AdClient::Close(void) {
	Reset();

	AdSocket::Close();
}

//-----------------------------------------------------------------------------
bool AdClient::Init(TCPsocket pSocket) {
	Reset();

	return AdSocket::Init(pSocket);
}

//-----------------------------------------------------------------------------
void AdClient::Reset(void) {
	questing = 0;
	i = j = 0;
	direc = 0;
	amt_wood = 0;
	timer_wood = 0;
}

//-----------------------------------------------------------------------------
void AdClient::ProcData(Uint8* pBuffer, Uint16& iOffset) {
	if (pBuffer == NULL) return;
	if (!IsActive()) return;

	uint16_t flag = *(uint16_t*)&pBuffer[iOffset];
	iOffset += sizeof(uint16_t);

	switch (flag) {
		case FLAG_WOOD_UPDATE: {
			uint16_t send_offset = 0;
			uint8_t send_data[MAX_PACKET];

			memcpy(send_data + send_offset, &flag, sizeof(uint16_t));
			send_offset += sizeof(uint16_t);
			memcpy(send_data + send_offset, &amt_wood, sizeof(uint8_t));
			send_offset += sizeof(uint8_t);

			SendData(send_data, send_offset);
		} break;

		case FLAG_WOOD_GETTIME: {
			uint16_t send_offset = 0;
			uint8_t send_data[MAX_PACKET];

			uint32_t time_left;
			if (questing) {
				time_left = WOOD_WAIT_TIME - (SDL_GetTicks() - timer_wood);
			} else time_left = 0;

			memcpy(send_data + send_offset, &flag, sizeof(uint16_t));
			send_offset += sizeof(uint16_t);
			memcpy(send_data + send_offset, &time_left, sizeof(uint32_t));
			send_offset += sizeof(uint32_t);

			SendData(send_data, send_offset);
		} break;

		case FLAG_WOOD_QUEST: {
			if (!questing) {
				questing = 1;
				timer_wood = SDL_GetTicks();
			}
		} break;

		case FLAG_PLAYER_POS: {
			i = *(uint8_t*)&pBuffer[iOffset];
			iOffset += sizeof(uint8_t);
			j = *(uint8_t*)&pBuffer[iOffset];
			iOffset += sizeof(uint8_t);
			direc = *(uint8_t*)&pBuffer[iOffset];
			iOffset += sizeof(uint8_t);
		} break;

		case FLAG_QUIT: {
			//running = 0;
			//printf("DB: shutdown by client id: %d\n", index);
		} break;
	}
}
