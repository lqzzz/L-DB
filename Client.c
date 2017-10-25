#include "Client.h"
#include"QueryProcessor\Scanner.h"
#include"QueryProcessor\Sqlparse.h"
void process_command(Client* c){
	c->token_head = scanner(c->errmsg, &c->command_iter, &c->comm_cnum, &c->comm_lnum);

	if(!c->token_head){

	}

	//sql_parse(c->errmsg,)

}
