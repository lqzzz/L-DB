#ifndef PARSE_QUE_H
#define PARSE_QUE_H
#include"Scanner.h"
#include"../BaseStruct/Pair.h"
#include"../BaseStruct/Vector.h"
#include"../Catalog.h" 
#include <stdio.h>
typedef struct con {
	enum Tokentype con_type; // WHERE SELECT JOIN INSERT
	struct con *right_con, *left_con;
	enum Tokentype operator_; // and or not  eq ex...
	Pair *left_opand, *right_opand;
	struct con* next_;
	//Relation* res_;
}Logicplan;

int parse_select(Arena *arena,DBnode* db,Token** curr);

#endif // !PARSE_QUE_H
