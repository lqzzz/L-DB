#include "../Mem/MemPool.h"
#include "Sqlparse.h"`
#include "../StorageEngine/BufferManager.h"
QueryNode* new_select_query(void){
	QueryNode* n = mem_alloc(sizeof(*n));
	n->type = SELECT;
	n->select_node = new_select_node();
	return n;
}

JoinNode* new_join_node(Table* t){
	JoinNode* n = mem_calloc(1,sizeof(JoinNode));
	n->table_ = t;
	return n;
}

JoinNode * join_lr(JoinNode* left, JoinNode* right){
	JoinNode* n = mem_calloc(1, sizeof(JoinNode));
	n->left = left;
	n->right = right;
	return n;
}



QueryNode* new_insert_query(){
	QueryNode* n = mem_alloc(sizeof(QueryNode));
	n->insert_node = new_insert_node();
	n->type = INSERT;
	return n;
}

DBitems* get_item(char* errmsg,DBnode* db,Token** curr,int flag) {
	DBitems *item;
	Table* t;
	Column* col;
	char* table_name;
	char* col_name;

	if (flag == BASE_ITEM) {
		item = new_dbitem();
		item->base_item = (*curr);
		NEXT_TOKEN;
		return item;
	}

	if (TOKEN_TYPE != ID)
		PARSE_ERROR("缺少ID");

	if (flag == FROM_ITEM) {
		table_name = (*curr)->value_;
		NEXT_TOKEN;
		if ((t = db_get_table(db, table_name)) == NULL)
			QUERY_ERROR("表名 %s 无效\n", table_name);
		item = new_dbitem();
		item->table_ = t;
		return item;
	}

	if (NEXT_TOKEN_TYPE == DOT) {
		table_name = (*curr)->value_;
		if ((t = db_get_table(db, table_name)) == NULL)
			QUERY_ERROR("表名 %s 无效\n", table_name);

		NEXT_TOKEN;
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID)
			PARSE_ERROR("缺少ID");
		col_name = (*curr)->value_;

		if ((col = table_get_col(t, col_name)) == NULL)
			QUERY_ERROR("列名 %s 无效\n", col_name);

		NEXT_TOKEN;
		item = new_dbitem();
		item->table_ = t;
		item->col_ = col;
		return item;
	}

	move_value(&(*curr)->value_, &col_name);
	NEXT_TOKEN;

	item = new_dbitem();
	item->col_name = col_name;
	return item;
ERROR:
	return NULL;
}

DBitems* new_dbitem(){
	DBitems* i = mem_alloc(sizeof(DBitems));
	LIST_INIT(&i->head);
	i->table_ = NULL;
	i->byname = NULL;
	i->base_item = NULL;
	return i;
}

void free_dbitem(DBitems* i) {
	if (i == NULL)
		return;
	if (i->byname)
		mem_free(i->byname);
	if (i->base_item)
		mem_free(i->base_item);
	mem_free(i);
}

void free_dbitem_list(DBitems* h) {
	LIST_DEL_ALL(&h->head, free_dbitem);
}


int get_item_list(char* errmsg,DBitems** ph, DBnode* db, Token** curr,int flag) {
	for (;;) {
		DBitems *item;
		if ((item = get_item(errmsg, db, curr,flag)) == NULL)
			return SQL_ERROR;
		DBitems_ADD(*ph, item);
		//可能sql已经结束
		if (TOKEN_TYPE != COMMA)
			return SQL_OK;

		NEXT_TOKEN;
	}
}

int check_item(char* errmsg, DBitems* checknode,DBitems* from){
	if (checknode == NULL)
		return SQL_OK;
	if (checknode->base_item)
		return SQL_OK;
	Table* t = checknode->table_;
	int hit_ = 0;
	if (t) {
		DBitems* curr;
		LIST_FOREACH(curr, from,
			if (t == curr->table_) {
				hit_ = 1;
				break;
			}
		);
		if (hit_ == 0) 
			QUERY_ERROR("表 %s 不在from语句内\n", TABLE_GET_NAME(t));	
	}else{
		char* col_name = checknode->col_name;
		DBitems* item;
		LIST_FOREACH(item, from,
			if (checknode->col_ = table_get_col(item->table_, col_name)) 
				hit_++;
		);

		if (hit_ == 0) QUERY_ERROR("列名 %s 无效\n", col_name);
		if (hit_ > 1) QUERY_ERROR("列名 %s 不明确\n", col_name);
	}
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

int check_item_list(char* errmsg, DBitems* checknode, DBitems* from) {
	DBitems *item;
	LIST_FOREACH(item, checknode,
		if (check_item(errmsg, item, from) == SQL_ERROR)
			return SQL_ERROR;
	);
	return SQL_OK;
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


//int execute_select(char * errmsg, DBnode * db, JoinNode* sel){
//		
//	return 0;
//}
//
//int execute_get_row(char * errmsg, DBnode * db, JoinNode* j) {
//	if(j->table_)
//		
//
//}
//
//int execute_select(char * errmsg, DBnode * db, JoinNode* sel) {
//
//}
//int 