#ifndef AE_H
#define AE_H
#include<WinSock2.h>
#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0

#define AE_READABLE 1
#define AE_WRITABLE 2

#define AE_FIRED 1
#define AE_NOT_FIRED 0

#define AE_AVAILBLE 1
#define AE_NOT_AVAILBLE 0

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_DONT_WAIT 4

#define AE_NOMORE -1
#define AE_NOTUSED(V) ((void) V)

struct aeEventLoop;

typedef void aeFileProc(struct aeEventLoop *eventLoop, SOCKET s, void *clientData, int mask);

typedef struct aeFileEvent {
	SOCKET s;
    int mask; 
    aeFileProc *rfileProc;
    aeFileProc *wfileProc;
    void *clientData;
} aeFileEvent;

typedef struct aeEventLoop {
    size_t setsize; 
    aeFileEvent *events; 
    size_t fesize;
    int stop;
    void *apidata; /* select()Êý¾Ý */
} aeEventLoop;

aeEventLoop *aeCreateEventLoop(int setsize);
void aeDeleteEventLoop(aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop,int eindex, SOCKET index, int mask,
        aeFileProc *proc, void *clientData);
void aeDeleteFileEvent(aeEventLoop *eventLoop, int index, int mask);
int aeGetFileEvents(aeEventLoop *eventLoop, int index);

void aeMain(aeEventLoop *eventLoop);
int aeProcessEvents(aeEventLoop *eventLoop);

int aeGetSetSize(aeEventLoop *eventLoop);
int aeResizeSetSize(aeEventLoop *eventLoop, int setsize);
#endif // !AE_H
