#include <stdio.h>
#include <stdlib.h>

#include "ae.h"
#include"ae_select.c"

aeEventLoop *aeCreateEventLoop(int setsize) {
	aeEventLoop *eventLoop;

	if ((eventLoop = malloc(sizeof(*eventLoop))) == NULL) goto err;

	eventLoop->setsize = setsize;

	eventLoop->stop = 0;

	eventLoop->fesize = 0;

	eventLoop->events = calloc(setsize,sizeof(aeFileEvent));

	if (eventLoop->events == NULL) goto err;
	if (aeApiCreate(eventLoop) == -1) goto err;

	return eventLoop;

err:
	if (eventLoop) {
		free(eventLoop->events);
		free(eventLoop);
	}
	return NULL;
}

int aeGetSetSize(aeEventLoop *eventLoop) {
	return eventLoop->setsize;
}

int aeResizeSetSize(aeEventLoop *eventLoop, int setsize) {

	if (setsize == eventLoop->setsize) return AE_OK;
	if (eventLoop->fesize >= setsize) return AE_ERR;
	if (aeApiResize(eventLoop,setsize) == -1) return AE_ERR;

	eventLoop->events = realloc(eventLoop->events,sizeof(aeFileEvent)*setsize);

	eventLoop->setsize = setsize;

	for (int i = eventLoop->fesize; i < setsize; i++)
		eventLoop->events[i].mask = AE_NONE;
	return AE_OK;
}

void aeDeleteEventLoop(aeEventLoop *eventLoop) {
	aeApiFree(eventLoop);
	free(eventLoop->events);
	free(eventLoop);
}

int aeCreateFileEvent(aeEventLoop *eventLoop, int eindex, SOCKET s, int mask,
	aeFileProc *proc, void *clientData) {

	if (eventLoop->fesize == eventLoop->setsize) {
		errno = ERANGE;
		return AE_ERR;
	}

	aeFileEvent *fe = eventLoop->events;
	int index = 0;

	if (eindex != -1) 
		fe += eindex;
	else {
		for (; fe->mask; fe++, index++);
		fe->s = s;
		fe->clientData = clientData;
	}

	if (aeApiAddEvent(eventLoop, fe->s, mask) == -1)
		return AE_ERR;
	eventLoop->fesize++;
	fe->mask |= mask;
	if (mask & AE_READABLE) fe->rfileProc = proc;
	if (mask & AE_WRITABLE) fe->wfileProc = proc;


	return index;
}

void aeDeleteFileEvent(aeEventLoop *eventLoop, int index, int mask)
{
	if (!eventLoop->fesize) return;

	aeFileEvent *fe = eventLoop->events + index;

	fe->mask == AE_NONE;

	eventLoop->fesize--;

	aeApiDelEvent(eventLoop, fe->s, mask);
}

int aeGetFileEvents(aeEventLoop *eventLoop, int index) {
	if (index >= eventLoop->setsize) return AE_ERR;
	return eventLoop->events[index].mask;
}

int aeProcessEvents(aeEventLoop *eventLoop){

	printf("开始处理事件\n");
	if (!eventLoop->fesize) return 0;

    aeApiState *state = eventLoop->apidata;

	memcpy(&state->_rfds,&state->rfds,sizeof(fd_set));
	memcpy(&state->_wfds,&state->wfds,sizeof(fd_set));

	select(eventLoop->setsize, &state->_rfds, &state->_wfds, NULL, NULL);

	int index = 0, numevents = 0;
	for (aeFileEvent* fe = eventLoop->events;
		index < eventLoop->setsize; index++, fe++) {

		SOCKET s = fe->s;

		//检测启用
		if (fe->mask == AE_NONE) continue;


		//检测激活套接字列表、可读可写
		if (fe->mask & AE_READABLE && FD_ISSET(s, &state->_rfds)) {
			fe->rfileProc(eventLoop, s, fe->clientData, fe->mask);
			numevents++;
		}else if (fe->mask & AE_WRITABLE && FD_ISSET(s, &state->_wfds)) {
			fe->wfileProc(eventLoop, s, fe->clientData, fe->mask);
			numevents++;
		}
		
	}
	return numevents;
}

void aeMain(aeEventLoop *eventLoop) {
	eventLoop->stop = 0;
	while (!eventLoop->stop) 
		// 开始处理事件
		aeProcessEvents(eventLoop);
}

