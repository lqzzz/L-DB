#ifndef _TOKENIZER_H
#define _TOKENIZER_H
#include "../Catalog.h"
#include"Scanner.h"
int sql_parse(char* errmsg,DBnode *dbnode, Token* tokenhead);
#endif // !_TOKENIZER_H
 