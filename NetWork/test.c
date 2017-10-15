#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

#include"ae.h"

int InitServer(void);
void AcceptTcpHandler(aeEventLoop *el, SOCKET s, void *privdata, int mask);
typedef struct Client {
	size_t eindex;
	char command[1024];
	char repyl[256];
	size_t sent_len;
	struct Client* next;
}Client;

struct Server {
	aeEventLoop* el;
	size_t sindex;
	char* bindaddr;
	char* port;
	int tcp_backlog;
	Client* clist;
	Client* curr_client;
};

struct Server server;

int main(void) {

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsadata;
	if (WSAStartup(sockVersion, &wsadata) != 0){
		return 0;
	}

	InitServer();
	aeMain(server.el);
}

void SendReplyToClient(aeEventLoop* el,SOCKET s,void* privdata,int mask) {

	Client* c = privdata;

	send(s, c->repyl, 1024, 0);
	aeDeleteFileEvent(el, c->eindex, AE_WRITABLE);
	aeDeleteFileEvent(el, c->eindex, AE_READABLE);
	printf("发送 %s 完成\n", c->repyl);
}

int PrepareClientToWrite(Client* c){
	if (aeCreateFileEvent(server.el, c->eindex,
		NULL, AE_WRITABLE, SendReplyToClient, c) == AE_ERR) {
		printf("tocw error\n");
		return -1;
	}
	return 1;
}

void AddReply(Client *c) {
	PrepareClientToWrite(c);
	strcpy(c->repyl, c->command);
}

int ProcessCommand(Client *c) {
	AddReply(c);
}

void ReadQueryFromClient(aeEventLoop *el, SOCKET s, void *privdata, int mask) {

	Client* c = privdata;

	server.curr_client = c;
	int len = recv(s, c->command, 1024, 0); 
	if (len > 0)
		printf("接收%d个字节\n", len);
	c->command[len] = '\0';
	ProcessCommand(c);

}

Client* CreateClient(SOCKET s) {

	Client* c = calloc(1, sizeof(Client));


	if ((c->eindex = aeCreateFileEvent(server.el, -1, s, AE_READABLE,
		ReadQueryFromClient, c)) == AE_ERR) {
		//关闭socket
		//free(client)
	}
	return c;
}

static void AcceptCommonHandler(SOCKET s) {
	Client* c = CreateClient(s);
	if (server.clist)
		c->next = server.clist;
	server.clist = c;
}

int InitServer(void) {
	server.bindaddr = "106.91.26.248";
	server.port = "8989";
	server.tcp_backlog = 16;
	server.clist = NULL;
	server.el = aeCreateEventLoop(16);

	struct addrinfo hints, *res;  
	int errcode;  

	memset (&hints, 0, sizeof (hints));  
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;  
	hints.ai_flags |= AI_PASSIVE;  

	errcode = getaddrinfo(server.bindaddr, server.port, &hints, &res);
	if (errcode != 0)  
	{  
		perror ("getaddrinfo");  
		return -1;  
	}  

	SOCKET s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (bind(s, res->ai_addr, res->ai_addrlen) == -1)
		return 0;
	if (listen(s, server.tcp_backlog) == -1)
		return 0;

	if ((server.sindex = aeCreateFileEvent(server.el, -1, s,
		AE_READABLE, AcceptTcpHandler, NULL)) == AE_ERR)
		printf("Unrecoverable error creating server.ipfd file event\n");

	freeaddrinfo(res);
}

static SOCKET AnetGenericAccept(SOCKET s, struct sockaddr *sa, socklen_t *len) {
	if ((s = accept(s, sa, len)) == -1) {
		printf("accept: %s", strerror(errno));
		return NULL;
	}
	return s;
}


SOCKET AnetTcpAccept(SOCKET so, char *ip, size_t ip_len, int *port) {
	SOCKET sc;
	struct sockaddr_storage sa;
	socklen_t salen = sizeof(sa);
	if ((sc = AnetGenericAccept(so, (struct sockaddr*)&sa, &salen)) == -1)
		return -1;
	if (sa.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&sa;
		if (ip) inet_ntop(AF_INET, (void*)&(s->sin_addr), ip, ip_len);
		if (port) *port = ntohs(s->sin_port);
	}
	return sc;
}

void AcceptTcpHandler(aeEventLoop *el, SOCKET s, void *privdata, int mask) {
	int cport;
	SOCKET cs;
	char cip[INET6_ADDRSTRLEN];

	if ((cs = AnetTcpAccept(s, cip, sizeof(cip), &cport)) == -1) {
		if (errno != EWOULDBLOCK)
			printf("Accepting client connection error");
		return;
	}
	printf("Accepted %s:%d", cip, cport);
	AcceptCommonHandler(cs);
}

