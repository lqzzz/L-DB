#include <string.h>
#include <WinSock2.h>
#include "ae.h"

typedef struct aeApiState {
    fd_set rfds, wfds;
    fd_set _rfds, _wfds;
} aeApiState;

static int aeApiCreate(aeEventLoop *eventLoop) {
    aeApiState *state = malloc(sizeof(aeApiState));

    if (!state) return -1;
    FD_ZERO(&state->rfds);
    FD_ZERO(&state->wfds);
    eventLoop->apidata = state;
    return 0;
}

static int aeApiResize(aeEventLoop *eventLoop, int setsize) {
    if (setsize >= FD_SETSIZE) return -1;
    return 0;
}

static void aeApiFree(aeEventLoop *eventLoop) {
    free(eventLoop->apidata);
}

static int aeApiAddEvent(aeEventLoop *eventLoop, SOCKET s, int mask) {
    aeApiState *state = eventLoop->apidata;

    if (mask & AE_READABLE) FD_SET(s,&state->rfds);
    if (mask & AE_WRITABLE) FD_SET(s,&state->wfds);
    return 0;
}

static void aeApiDelEvent(aeEventLoop *eventLoop, SOCKET s, int mask) {
    aeApiState *state = eventLoop->apidata;

    if (mask & AE_READABLE) FD_CLR(s,&state->rfds);
    if (mask & AE_WRITABLE) FD_CLR(s,&state->wfds);
}

static int aeApiPoll(aeEventLoop *eventLoop) {
    aeApiState *state = eventLoop->apidata;
    int retval, numevents = 0;

    memcpy(&state->_rfds,&state->rfds,sizeof(fd_set));
    memcpy(&state->_wfds,&state->wfds,sizeof(fd_set));

	retval = select(eventLoop->setsize,
		&state->_rfds, &state->_wfds, NULL, NULL);
    if (retval > 0) {
		int index = 0;
		for (aeFileEvent* fe = eventLoop->events;
			index < eventLoop->setsize; index++, fe++) {
            int mask = 0;
            if (fe->mask == AE_NONE) continue;

			SOCKET s = fe->s;
            if (fe->mask & AE_READABLE && FD_ISSET(s,&state->_rfds))
                mask |= AE_READABLE;
            if (fe->mask & AE_WRITABLE && FD_ISSET(s,&state->_wfds))
                mask |= AE_WRITABLE;
			fe->mask = mask;
            numevents++;
        }
    }
    return numevents;
}
