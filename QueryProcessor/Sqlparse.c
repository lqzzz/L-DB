#include "Scanner.h"
#include "Relation.h"
#include "../BaseStruct/Listhead.h"
#include "../Catalog.h"
#include "../BaseStruct/Vector.h"
#include "../Mem/MemPool.h"
#include "../BaseStruct/Pair.h"
#include "../StorageEngine/BufferManager.h"
#include"Sqlparse.h"`

QueryNode * new_select_query(SelectNode* select){
	QueryNode* n = mem_alloc(sizeof(*n));
	n->type = SELECT;
	n->select_node = select;
	return n;
}

QueryNode* new_select_node(){
	QueryNode* n = mem_alloc(sizeof(*n));

	n->con_type = SELECT;

	return n;
}

void dbitem_add(DBitem* head, DBitem * addnode){
	
}

QueryNode* new_insert_query(InsertNode* insert){
	QueryNode* n = mem_alloc(sizeof(*n));
	n->type = INSERT;
	n->insert_node = insert;
	return n;
}

int get_query_head(char* errmsg,DBnode* db,DBitem* item,Token** curr) {
	if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");

	if (NEXT_TOKEN_TYPE == DOT) {
		move_value(&(*curr)->value_, &item->tablename);
		NEXT_TOKEN;
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
	}
	move_value(&(*curr)->value_, &item->colname);

	NEXT_TOKEN;
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

int check_dbitem(char* errmsg, DBnode* db, DBitem* item, Vector* vfrom){
	Table* from_table;
	char* table_name = item->tablename;
	int hit_ = 0;
	if (table_name == NULL) {
		char* col_name = item->colname;

		VECTOR_FOREACH(from_table, vfrom,
			
			if (table_get_col(from_table,col_name) != NULL)
				hit_++;
			);
		if (hit_ == 0) QUERY_ERROR("列名 %s 无效\n", col_name);
		if (hit_ > 1) QUERY_ERROR("列名 %s 不明确\n", col_name);
	}else{
		VECTOR_FOREACH(from_table, vfrom,
			if (strcmp(table_name, from_table->t_info.table_name) == 0)
				hit_ = 1;
			);
		if (hit_ == 0) 
			QUERY_ERROR("表名 %s 无效\n", table_name);	
	}
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

int get_schema(char* errmsg,DBnode* db,Pair* sch,Token** curr) {
	if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
	if (NEXT_TOKEN_TYPE == DOT) {
		Table* target_table;
		if ((target_table = db_get_table(db, (*curr)->value_)) == NULL) PARSE_ERROR("表名无效");
		move_value(&(*curr)->value_, &sch->first_);
		NEXT_TOKEN;
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
		if (table_get_col(target_table, (*curr)->value_) == NULL) PARSE_ERROR("列名无效");
		move_value(&(*curr)->value_, &sch->second_);
	}else 
		move_value(&(*curr)->value_, &sch->second_);

	NEXT_TOKEN;
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

int check_schema(char* errmsg, DBnode* db, Pair* p, Vector* vfrom){
	if (p == NULL || PairGetSecond(p) == NULL)
		return SQL_OK;
	Table* from_table;
	char* table_name = PairGetFirst(p);
	int hit_ = 0;
	if (table_name == NULL) {
		char* col_name = PairGetSecond(p);
		VECTOR_FOREACH(from_table, vfrom,
			
			if (table_get_col(from_table,col_name) != NULL)
				hit_++;
			);
		if (hit_ == 0) QUERY_ERROR("列名 %s 无效\n", col_name);
		if (hit_ > 1) QUERY_ERROR("列名 %s 不明确\n", col_name);
	}else{
		VECTOR_FOREACH(from_table, vfrom,
			if (strcmp(table_name, from_table->t_info.table_name) == 0)
				hit_ = 1;
			);
		if (hit_ == 0) 
			QUERY_ERROR("表名 %s 无效\n", table_name);	
	}
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}


int sql_parse(char* errmsg,DBnode *db, Token* token_head,QueryNode** pnode) {
	Token* t_curr = token_head;
	Token** curr = &t_curr;
	DBnode* dbnode = db;
	do {
		switch (TOKEN_TYPE)
		{
		case USE:
			if (MOVE_NEXT_TOKEN_TYPE != ID)
				PARSE_ERROR("缺少ID");
			if ((dbnode = DBNODE_SEARCH(db, (*curr)->value_)) == NULL)
				PARSE_ERROR("数据库不存在");
			NEXT_TOKEN;
			break;
		case CREATE:
			NEXT_TOKEN;
			if (parse_create(errmsg,dbnode, curr) == SQL_ERROR)
				goto ERROR;
			break;
		case SELECT:
			NEXT_TOKEN;
			if (parse_select(errmsg, dbnode, curr, pnode) == -1);
				goto ERROR;
			break;
		case INSERT:
			if (MOVE_NEXT_TOKEN_TYPE != INTO) PARSE_ERROR("缺少INTO");
			if (MOVE_NEXT_TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
			if (parse_insert(errmsg,dbnode, curr,pnode) == -1) goto ERROR;
			break;
		case UPDATE:
			break;
		default:
			break;
		}
	} while (t_curr != token_head);
	return SQL_OK;
ERROR: 
	return SQL_ERROR;
}


