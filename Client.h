#ifndef _CLIENT_H
#define _CLIENT_H
#include"BaseStruct\Listhead.h"
#include"QueryProcessor\Scanner.h"
#include"QueryProcessor\Sqlparse.h"

#define COMMAND 0;
#define DATASET 1;

typedef struct {
	Listhead head;
	size_t eindex;
	char* command_str;
	char* command_iter;
	int comm_cnum;
	int comm_lnum;
	Token* token_head;
	QueryNode* quenode;
	char errmsg[128];
}Client;

void process_command(Client* c);


#endif // !_CLIENT_H
