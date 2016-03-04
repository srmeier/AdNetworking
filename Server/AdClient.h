#ifndef __ADCLIENT_H_
#define __ADCLIENT_H_

#include "AdDefine.h"
#include "AdSocket.h"

//-----------------------------------------------------------------------------
class AdClient : public AdSocket {
public:
	int questing;
	uint8_t i, j;
	uint8_t direc;
	uint8_t amt_wood;
	uint32_t timer_wood;

public:
	virtual void Close(void);
	virtual bool Init(TCPsocket pSocket);
	virtual void Reset(void);

public:
	AdClient(void);
	virtual ~AdClient(void);
};

#endif
