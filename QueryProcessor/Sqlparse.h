#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <stdio.h>

#include "../Catalog.h"
#include"Scanner.h"
#include"../BaseStruct/Pair.h"
#include"../StorageEngine/Page.h"

#define SQL_ERROR -1
#define SQL_OK 1

#define PARSE_ERROR(buf) do{\
sprintf(errmsg, " %d ÐÐ, %d ÁÐ ´íÎó£º%s\n", (*curr)->l_num, (*curr)->c_num, buf); goto ERROR;\
}while(0)

#define QUERY_ERROR(buf,var) do{\
sprintf(errmsg, buf,var); goto ERROR;\
}while(0)

#define DBITEM_ADD(_head,_item) \
	if ((_head) == NULL)\
		_head = _item; \
	else LIST_ADD_TAIL(&(_head)->head, &(_item)->head) \


typedef struct insert InsertNode;
typedef struct select SelectNode;
typedef struct join JoinNode;
typedef struct condition WhereNode;
typedef struct query QueryNode;
typedef struct selectitem;

typedef struct {
	Listhead head;
	
	char* tablename;
	char* colname;
	enum Tokentype function_type;
	char* byname;

	Table* from_table;
}DBitem;

typedef struct join{
	char* table_name;
	JoinNode *left, *right;
}JoinNode;

typedef struct condition{
	enum Tokentype operator_; // and or not  eq ex...
	DBitem left_opand, right_opand;
	WhereNode *left, *right;
	JoinNode *join_node;
	struct select *sub_query;
}WhereNode;


typedef struct {
	int is_del;
	char* table_name;
	Vector dest_src_pairs;
	WhereNode* condition;
}UpdateNode;

typedef struct query{
	enum Tokentype type; 

	union {
		InsertNode* insert_node;
		SelectNode* select_node;
		UpdateNode* update_node;
	};


	union {
		Vector insert_rows,select_cols;
	};

	enum Tokentype con_type; // WHERE SELECT JOIN INSERT
	QueryNode *left_con, *right_con, *rows_node;

	enum Tokentype operator_; // and or not  eq ex...
	Pair left_opand, right_opand;

	FHead* file_;
}QueryNode;

void dbitem_add(DBitem* head, DBitem* addnode);
QueryNode* new_insert_query(InsertNode* insert);
//DBitem* get_from(char* errmsg,DBnode* dbnode,Token** curr);

void* create_con(int oper, void* leftcon, void* rightcon);
void con_del(struct con* con);
//QueryNode* new_insert_node();
QueryNode* new_select_query();
QueryNode* new_join_node();

void* create_join(char* errmsg,Table* table, Vector* from);
void* get_con_exp(DBnode* db, Token** curr);
void* get_term(DBnode* db,Token** curr);
void* get_factor(DBnode* db,Token** curr);
int get_base_exp(DBnode* db, Pair* p, Token** curr);
int check_con(DBnode* db, struct con* con,Vector* vfrom);
int get_schema(char* errmsg, DBnode* db, Pair* sch, Token** curr);
int check_schema(char* errmsg, DBnode* db, Pair* p, Vector* vfrom);
void* create_join(Table* table, Vector* from);

int sql_parse(char* errmsg,DBnode *dbnode, Token* tokenhead,QueryNode** node);

int parse_select(char* errmsg,DBnode* db, Token** curr,QueryNode** qnode);
int parse_create(char* errmsg,DBnode* dbnode, Token** curr);
Table* parse_create_table(char* errmsg,DBnode *dbnode,Token** token);
int parse_create_column(char* errmsg,Table* t,Token** token);
int parse_datatype(char* errmsg,int datatype, Token**);
int parse_insert(char* errmsg, DBnode* dbnode, Token** curr, QueryNode** pnode);
int parse_insert_values(char* errmsg, DBnode* db, Table* table, Vector* collist, Token** curr, QueryNode** pnode);

#endif // !_TOKENIZER_H
 