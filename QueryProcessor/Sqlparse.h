#ifndef _TOKENIZER_H
#define _TOKENIZER_H
#include "../Catalog.h"
#include"Scanner.h"
#include"../BaseStruct/Pair.h"
typedef struct con {
	Vector inset_rows;
	VectorIter iter;
	enum Tokentype con_type; // WHERE SELECT JOIN INSERT
	struct con *right_con, *left_con;
	enum Tokentype operator_; // and or not  eq ex...
	Pair *left_opand, *right_opand;
	struct con* next_;
}Logicplan;
int sql_parse(char* errmsg,DBnode *dbnode, Token* tokenhead);
#endif // !_TOKENIZER_H
 