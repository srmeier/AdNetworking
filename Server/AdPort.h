#ifndef __ADPORT_H_
#define __ADPORT_H_

#include "AdDefine.h"
#include "AdSocket.h"

//-----------------------------------------------------------------------------
typedef std::list<int> SidList;
typedef std::queue<int> SidQueue;

//-----------------------------------------------------------------------------
class AdPort {
protected:
	SDL_Thread* m_pSendThread;
	SDL_Thread* m_pAcceptThread;

	int m_iNumOfWorkers;
	SDL_Thread* m_pWorkerThreads[MAX_USERS];

public:
	SDL_mutex* m_pSendMutex;
	AdSocket m_pSockets[MAX_USERS];

	SidList m_listSid;
	SidQueue m_queueRecvSid;
	TCPsocket m_pListenSocket;

	SDL_sem* m_pSendSem;
	SDL_sem* m_pWorkerSem;

	SDL_atomic_t m_aCloseWorkers;
	SDL_atomic_t m_aCloseSendThread;
	SDL_atomic_t m_aCloseAcceptThread;

public:
	AdPort(void);
	~AdPort(void);
};

#endif
