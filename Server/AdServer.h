#ifndef __ADSERVER_H_
#define __ADSERVER_H_

#include "AdDefine.h"
#include "AdClient.h"

//-----------------------------------------------------------------------------
class AdServer {
protected:
	int m_iNextInd;
	TCPsocket m_pServerSocket;
	SDLNet_SocketSet m_pSocketSet;
	AdClient m_pClients[MAX_SOCKETS];

public:
	void CloseSocket(int iIndex);
	bool AcceptSocket(int iIndex);
	Uint8* RecvData(int iIndex, Uint16& sLength);
	int CheckSockets(void);
	void UpdateClients(void);
	void ProcPackets(int& iNumReady);
	bool CheckForNewConnection(void);

public:
	AdServer(void);
	~AdServer(void);
};

#endif
