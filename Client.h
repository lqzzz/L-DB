#ifndef _CLIENT_H
#define _CLIENT_H
#include"BaseStruct\Listhead.h"

#define COMMAND 0;
#define DATASET 1;

typedef struct {
	char* buf;
	int c_num;
	int l_num;
}StreamBuff;

typedef struct {
	Listhead head;
	size_t eindex;
	StreamBuff* recvbuf;
	StreamBuff* sendbuf;

}Client;

void process_command(Client* c);


#endif // !_CLIENT_H
