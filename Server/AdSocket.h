#ifndef __ADSOCKET_H_
#define __ADSOCKET_H_

#include "AdDefine.h"

//-----------------------------------------------------------------------------
class AdSocket {
protected:
	TCPsocket m_pSocket;

public:
	bool IsActive(void) { return (m_pSocket != NULL); }
	int RemoveFromSocketSet(SDLNet_SocketSet pSocketSet);
	virtual void Close(void);
	virtual bool Init(TCPsocket pSocket);
	int AddToSocketSet(SDLNet_SocketSet pSocketSet);
	int SendData(Uint8* pBuffer, int iOffset);
	int RecvData(Uint8* pBuffer, int iSize);
	bool IsSocketReady(void);
	virtual void ProcData(Uint8* pBuffer, Uint16& iOffset) = 0;

public:
	AdSocket(void);
	~AdSocket(void);
};

#endif
