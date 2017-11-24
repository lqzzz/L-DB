#ifndef _SCANNER_H
#define _SCANNER_H
#include<stdint.h>
#include "../BaseStruct/Listhead.h"
#define TOKEN_TYPE (*curr)->token_type
#define NEXT_TOKEN LIST_MOVE_NEXT(curr) 

#define NEXT_TOKEN_TYPE ((Token*)LIST_GET_NEXT(*curr))->token_type
//#define NEXT_TOKEN_TYPE LIST_ENTRY() 
//#define LIST_ENTRY(ptr,type,member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))  
#define MOVE_NEXT_TOKEN_TYPE ((Token*)NEXT_TOKEN)->token_type 
typedef struct {
	Listhead list_head;
	enum Tokentype token_type;
	int c_num;
	int l_num;
	void *value_;
}Token;

void init_key_word();
void token_del(Token* token);
Token* scanner(char* errmsg, char** pcommandstr, int* c_num, int* l_num);
void move_value(void** src, void** dest);

#endif // !_SCANNER_H
