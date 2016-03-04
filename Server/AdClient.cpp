/*
*/

#include "AdClient.h"

//-----------------------------------------------------------------------------
AdClient::AdClient(void) {
	Reset();

	AdSocket::AdSocket();
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
