#ifndef __ADSOCKET_H_
#define __ADSOCKET_H_

#include "AdDefine.h"

//-----------------------------------------------------------------------------
class AdSocket {
protected:
	int m_iSid;

public:
	SDL_mutex* m_pMutex;
	TCPsocket  m_tcpSocket;

public:
	int  GetSocketID(void)     {return m_iSid;}
	void SetSocketID(int iSid) {m_iSid = iSid;}

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
