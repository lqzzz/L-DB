#include "../Mem/MemPool.h"
#include"Sqlparse.h"`

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
	char* table_name = (*curr)->value_;

	if (TOKEN_TYPE != ID) 
		PARSE_ERROR("ȱ��ID");

	if ((t = db_get_table(db, table_name)) == NULL)
		QUERY_ERROR("���� %s ��Ч\n", table_name);

	if (flag == FROM_ITEM) {
		item = mem_alloc(sizeof(DBitems));
		item->table_ = t;
		return item;
	}

	if (flag == INSERT_ITEM) {
		item = mem_alloc(sizeof(DBitems));
		move_value(&(*curr)->value_, &item->insert_item);
		return item;
	}

	if (NEXT_TOKEN_TYPE == DOT) {
		NEXT_TOKEN;
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID) 
			PARSE_ERROR("ȱ��ID");
	}

	if ((col = table_get_col(t, (*curr)->value_)) == NULL)
		QUERY_ERROR("���� %s ��Ч\n", (*curr)->value_);

	NEXT_TOKEN;

	item = mem_alloc(sizeof(DBitems));
	item->table_ = t;
	item->col_ = col;
	return item;
ERROR:
	return NULL;
}

int get_item_list(char* errmsg,DBitems** ph, DBnode* db, Token** curr,int flag) {
	for (;;) {
		DBitems *item;
		if ((item = get_item(errmsg, db, curr,flag)) == NULL)
			return SQL_ERROR;
		DBitems_ADD(*ph, item);
		//����sql�Ѿ�����
		if (TOKEN_TYPE != COMMA)
			return SQL_OK;

		NEXT_TOKEN;
	}
}

int check_item(char* errmsg, DBitems* checknode,DBitems* from){
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
			QUERY_ERROR("�� %s ����from�����\n", TABLE_GET_NAME(t));	
	}else{
		char* col_name = checknode->col_->column_name;
		DBitems* item;
		LIST_FOREACH(item, from,
			if (table_get_col(from->table_, col_name) != NULL) 
				hit_++;
		);
		if (hit_ == 0) QUERY_ERROR("���� %s ��Ч\n", col_name);
		if (hit_ > 1) QUERY_ERROR("���� %s ����ȷ\n", col_name);
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
				PARSE_ERROR("ȱ��ID");
			if ((dbnode = DBNODE_SEARCH(db, (*curr)->value_)) == NULL)
				PARSE_ERROR("���ݿⲻ����");
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
			if (MOVE_NEXT_TOKEN_TYPE != INTO) PARSE_ERROR("ȱ��INTO");
			if (MOVE_NEXT_TOKEN_TYPE != ID) PARSE_ERROR("ȱ��ID");
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


