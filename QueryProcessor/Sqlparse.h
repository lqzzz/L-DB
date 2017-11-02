#ifndef _TOKENIZER_H
#define _TOKENIZER_H
#include "../Catalog.h"
#include"Scanner.h"
#include"../BaseStruct/Pair.h"
#include"../StorageEngine/Page.h"
typedef struct{
	Vector inset_rows;
	VectorIter iter_;
	enum Tokentype con_type; // WHERE SELECT JOIN INSERT
	struct con  *left_con, *right_con;
	enum Tokentype operator_; // and or not  eq ex...
	Pair left_opand, right_opand;
	struct con* next_;
	FHead* file_;
}QueryNode;

int sql_parse(char* errmsg,DBnode *dbnode, Token* tokenhead,QueryNode** node);
#endif // !_TOKENIZER_H
 