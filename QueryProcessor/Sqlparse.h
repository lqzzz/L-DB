#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <stdio.h>

#include "../Catalog.h"
#include"Scanner.h"
#include"../BaseStruct/Pair.h"
#include"../StorageEngine/Page.h"
#include"../Mem/obj.h"

#define SQL_ERROR -1
#define SQL_OK 1

#define FROM_ITEM 1
#define TAB_COL_ITEM 2
#define BASE_ITEM 3


#define PARSE_ERROR(buf) do{\
sprintf(errmsg, " %d 行, %d 列 错误：%s\n", (*curr)->l_num, (*curr)->c_num, buf); goto ERROR;\
}while(0)

#define QUERY_ERROR(buf,var) do{\
sprintf(errmsg, buf,var); goto ERROR;\
}while(0)

#define DBitems_ADD(_head,_item) \
	if ((_head) == NULL)\
		_head = _item; \
	else LIST_ADD_TAIL(&(_head)->head, &(_item)->head) \


typedef struct insert InsertNode;
typedef struct select SelectNode;
typedef struct join JoinNode;
typedef struct condition WhereNode;
typedef struct query QueryNode;
//typedef struct selectitem;


typedef struct {
	Listhead head;

	Table* table_;
	Column* col_;
	char* col_name;
	enum Tokentype function_type;
	char* byname;
	Token* base_item;
}DBitems;

typedef struct select{
	DBitems *select_items, *from_items, *group_by_items;
	WhereNode* condition;
	WhereNode* having;
	JoinNode* join;
}SelectNode;

typedef struct insert{
	DBitems *set_cols, *insert_fields, *table_item;
	char* insert_row;
	SelectNode* select_node;
}InsertNode;

typedef struct join{
	Table* table_;
	JoinNode *left, *right;
	obj* row_obj;
	FHead* file;
	int pid;
	int rid;
}JoinNode;

typedef struct condition{
	int res_type;
	enum Tokentype operator_; // and or not  eq ex...
	DBitems *opand; // 基本表达式使用
	WhereNode *left, *right; 
	SelectNode* sub_query;
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


	//Vector insert_rows, select_cols; 
	//enum Tokentype con_type; // WHERE SELECT JOIN INSERT
	//QueryNode *left_con, *right_con, *rows_node;

	//enum Tokentype operator_; // and or not  eq ex...
	//Pair left_opand, right_opand;

	//FHead* file_;
}QueryNode;

QueryNode* new_insert_query(void);
QueryNode* new_select_query(void);
InsertNode* new_insert_node(void);
SelectNode* new_select_node(void);
JoinNode* new_join_node(Table* t);
JoinNode* join_lr(JoinNode* left, JoinNode* right);

DBitems* new_dbitem();
void free_dbitem_list(DBitems* h);
void free_dbitem(DBitems* i);

int get_item_list(char* errmsg, DBitems** ph, DBnode* db, Token** curr, int isfrom);
DBitems* get_item(char* errmsg, DBnode* db, Token** curr, int isfrom);
int check_item_list(char* errmsg, DBitems* checknode, DBitems* from);
int check_item(char* errmsg, DBitems* checknode, DBitems* from);

int sql_parse(char* errmsg,DBnode *dbnode, Token* tokenhead,QueryNode** node);

int parse_create(char* errmsg,DBnode* dbnode, Token** curr);
Table* parse_create_table(char* errmsg,DBnode *dbnode,Token** token);
int parse_create_column(char* errmsg,Table* t,Token** token);
int parse_datatype(char* errmsg,int datatype, Token**);

int parse_select(char* errmsg,DBnode* db, Token** curr,QueryNode** qnode);
int parse_insert(char* errmsg, DBnode* dbnode, Token** curr, QueryNode** pnode);
WhereNode* parse_where(char* errmsg, DBnode* db, Token** curr, DBitems* itab);

int execute_select(char* errmsg,DBnode* db,SelectNode* sel);


#endif // !_TOKENIZER_H
 