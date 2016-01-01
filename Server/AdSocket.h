#ifndef __ADSOCKET_H_
#define __ADSOCKET_H_

#include "AdDefine.h"

//-----------------------------------------------------------------------------
class AdSocket {
public:
	SDL_mutex* m_pMutex;
	TCPsocket  m_tcpSocket;

public:
	void         Init(TCPsocket pSocket);
	virtual void ProcData(uint8_t* pData, uint16_t* sOffset);
	int          SendData(uint8_t* pData, int iLen, uint16_t sFlag);
	uint8_t*     RecvData(int* iLen);

public:
	AdSocket(void);
	~AdSocket(void);
};

#endif
