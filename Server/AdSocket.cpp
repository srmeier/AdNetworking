/*
*/

#include "AdSocket.h"

//-----------------------------------------------------------------------------
AdSocket::AdSocket(void) {
	m_pSocket = NULL;
}

//-----------------------------------------------------------------------------
AdSocket::~AdSocket(void) {
	Close();
}

//-----------------------------------------------------------------------------
int AdSocket::RemoveFromSocketSet(SDLNet_SocketSet pSocketSet) {
	return SDLNet_TCP_DelSocket(pSocketSet, m_pSocket);
}

//-----------------------------------------------------------------------------
void AdSocket::Close(void) {
	if (m_pSocket) SDLNet_TCP_Close(m_pSocket);
	m_pSocket = NULL;
}

//-----------------------------------------------------------------------------
bool AdSocket::Init(TCPsocket pSocket) {
	m_pSocket = SDLNet_TCP_Accept(pSocket);
	return IsActive();
}

//-----------------------------------------------------------------------------
int AdSocket::AddToSocketSet(SDLNet_SocketSet pSocketSet) {
	return SDLNet_TCP_AddSocket(pSocketSet, m_pSocket);
}

//-----------------------------------------------------------------------------
int AdSocket::SendData(Uint8* pBuffer, int iOffset) {
	if (!IsActive()) return 0;
	return SDLNet_TCP_Send(m_pSocket, pBuffer, iOffset);
}

//-----------------------------------------------------------------------------
int AdSocket::RecvData(Uint8* pBuffer, int iSize) {
	if (!IsActive()) return 0;
	return SDLNet_TCP_Recv(m_pSocket, pBuffer, iSize);
}

//-----------------------------------------------------------------------------
bool AdSocket::IsSocketReady(void) {
	return (SDLNet_SocketReady(m_pSocket) & 0x01);
}
