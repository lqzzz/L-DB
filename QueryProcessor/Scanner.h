#ifndef _SCANNER_H
#define _SCANNER_H
#include<stdint.h>
#include "Listhead.h"
#include "SourceStream.h"
#define TOKEN_TYPE (*curr)->token_type
#define NEXT_TOKEN LIST_MOVE_NEXT(curr) 
#define NEXT_TOKEN_TYPE ((Token*)LIST_GET_NEXT(*curr))->token_type
#define MOVE_NEXT_TOKEN_TYPE ((Token*)NEXT_TOKEN)->token_type 
#define PARSE_ERROR(buf) do{\
fprintf(stderr, " %d ÐÐ, %d ÁÐ ´íÎó£º%s\n", (*curr)->l_num, (*curr)->c_num, buf); goto ERROR;\
}while(0)
typedef struct {
	Listhead list_head;
	enum Tokentype token_type;
	uint16_t c_num;
	uint16_t l_num;
	void *value_;
}Token;
void init_key_word();
void token_del(Token* token);
Token* scanner(Srcstream* stream);
Token* get_next_token(Srcstream* stream);
void move_value(void** src, void** dest);
#endif // !_SCANNER_H
