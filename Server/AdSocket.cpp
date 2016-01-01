/*
*/

#include "AdSocket.h"

//-----------------------------------------------------------------------------
AdSocket::AdSocket(void) {
	m_tcpSocket = NULL;
	m_pMutex = SDL_CreateMutex();
}

//-----------------------------------------------------------------------------
AdSocket::~AdSocket(void) {
	m_tcpSocket = NULL;
	SDL_DestroyMutex(m_pMutex);
}

//-----------------------------------------------------------------------------
void AdSocket::Init(TCPsocket pSocket) {
	m_tcpSocket = pSocket;
}

//-----------------------------------------------------------------------------
void AdSocket::ProcData(uint8_t* pData, uint16_t* sOffset) {
	/*
	if(m_tcpSocket==NULL || pData==NULL) return;

	uint16_t flag = *(uint16_t*) &pData[*sOffset];
	*sOffset += sizeof(uint16_t);

	switch(flag) {

	}
	*/
}

//-----------------------------------------------------------------------------
int AdSocket::SendData(uint8_t* pData, int iLen, uint16_t sFlag) {
	if(m_tcpSocket == NULL) return -1;

	int offset = 0;
	uint8_t data[MAX_PACKET];

	memcpy(data+offset, &sFlag, sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(data+offset, pData, iLen);

	int num_sent;
	SDL_LockMutex(m_pMutex);
	num_sent = SDLNet_TCP_Send(m_tcpSocket, data, offset);	
	SDL_UnlockMutex(m_pMutex);

	return (num_sent-sizeof(uint16_t));
}

//-----------------------------------------------------------------------------
uint8_t* AdSocket::RecvData(int* iLen) {
	if(m_tcpSocket == NULL) return NULL;

	int offset = 0;
	uint8_t data[MAX_PACKET];

	SDL_LockMutex(m_pMutex);
	*iLen = SDLNet_TCP_Recv(m_tcpSocket, data, MAX_PACKET);
	SDL_UnlockMutex(m_pMutex);

	if(*iLen <= 0) return NULL;

	uint8_t* pData = (uint8_t*) malloc(*iLen*sizeof(uint8_t));
	memcpy(pData, data, *iLen);

	return pData;
}
