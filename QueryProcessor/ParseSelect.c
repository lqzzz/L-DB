#include"Sqlparse.h"
#include"../BaseStruct/Listhead.h"

typedef struct select{
	DBitem *select_items, *from_items, *group_by_items;
	WhereNode* condition;
	WhereNode* having;
	JoinNode* join;

}SelectNode;

SelectNode* new_select_node() {

}

static DBitem* get_select_item(char* errmsg,DBnode* db,Token** curr) {

	if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");

	char *col_name = NULL, *table_name = NULL;

	if (NEXT_TOKEN_TYPE == DOT) {
		move_value(&(*curr)->value_, &table_name);
		NEXT_TOKEN;
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
	}
	move_value(&(*curr)->value_, &col_name);

	NEXT_TOKEN;

	DBitem *item = mem_alloc(sizeof(DBitem));
	item->tablename = table_name;
	item->colname = col_name;

	return item;
ERROR:
	return NULL;
}

int get_select_head(char* errmsg,DBitem** ph, DBnode* db, Token** curr) {
	for (;;) {
		DBitem *item;
		if ((item = get_select_item(errmsg, db, curr)) == NULL)
			return SQL_ERROR;
		DBITEM_ADD(*ph, item);

		if (TOKEN_TYPE != COMMA)
			return SQL_OK;
		NEXT_TOKEN;
	}
}

DBitem* get_from_item(char* errmsg, DBnode* db, Token** curr) {
	if (TOKEN_TYPE != ID) 
		PARSE_ERROR("缺少 ID ");
	Table* t;
	if ((t = db_get_table(db, (*curr)->value_)) == NULL)
		PARSE_ERROR(" FROM 语句 表名无效");

	DBitem *item = mem_alloc(sizeof(DBitem));
	item->from_table = t;
	return item;
ERROR:
	return NULL;
}

int get_from(char* errmsg, DBitem** ph, DBnode* db, Token** curr) {
	for (;;) {
		DBitem *item;
		if ((item = get_from_item(errmsg, db, curr)) == NULL)
			return SQL_ERROR;
		DBITEM_ADD(*ph, item);
		//可能sql已经结束
		if (MOVE_NEXT_TOKEN_TYPE != COMMA)
			return SQL_OK;
		NEXT_TOKEN;
	}
}



int check_item(char* errmsg, DBnode* db, DBitem* checknode,DBitem* from){

	char* table_name = checknode->tablename;
	int hit_ = 0;
	if (table_name == NULL) {
		char* col_name = checknode->colname;

		DBitem* item;
		LIST_FOREACH(item, from,
			if (table_get_col(item->from_table,col_name) != NULL)
				hit_++;
		);

		if (hit_ == 0) QUERY_ERROR("列名 %s 无效\n", col_name);
		if (hit_ > 1) QUERY_ERROR("列名 %s 不明确\n", col_name);
	}else{

		DBitem* curr;
		LIST_FOREACH(curr,from,
			if(strcmp(table_name,curr->tablename) == 0)
				hit_ = 1;
			);

		if (hit_ == 0) 
			QUERY_ERROR("表名 %s 无效\n", table_name);	
	}
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

int check_sel_head(char* errmsg, DBnode* db, DBitem* checknode, DBitem* from) {
	DBitem *item;
	LIST_FOREACH(item, checknode,
		if (check_item(errmsg, db, item, from) == SQL_ERROR)
			return SQL_ERROR;
	);
	return SQL_OK;
}

int parse_select(char* errmsg,DBnode* db, Token** curr,QueryNode** qnode){
	SelectNode* sel_node = mem_calloc(1,sizeof(SelectNode));

	if (TOKEN_TYPE == STAR)
		NEXT_TOKEN;
	else if ( TOKEN_TYPE == ID && 
			get_select_head(errmsg, &sel_node->select_items, 
			db, curr) == SQL_ERROR) {

		return SQL_ERROR;
	}else 
		PARSE_ERROR("缺少 ID 或 *");

	if (TOKEN_TYPE != FROM) 
		PARSE_ERROR("缺少FROM");
	NEXT_TOKEN;

	if (get_from(errmsg, &sel_node->from_items, db, curr) == SQL_ERROR)
		return SQL_ERROR;

	if (check_sel_head(errmsg, db, sel_node->select_items, sel_node->from_items) == SQL_ERROR)
		return SQL_ERROR;


	struct con* join_con = create_join(&v_from);

	if (TOKEN_TYPE == WHERE) {
		NEXT_TOKEN;
		if (((con_ = get_con_exp(db, curr)) == NULL || check_con(db, con_, &v_from) == -1))
			goto ERROR;
		con_->next_ = join_con;
	}else con_ = join_con;

	return sel_node;
ERROR:
	mem_free(sel_node);
	return NULL;
}

void* create_join(char* errmsg,Vector* from) {
	Table* table_;
	struct con* con_ = NULL;	
	VECTOR_FOREACH(table_, from, 	
		QueryNode* new_con = mem_calloc(1, sizeof(*new_con));
		size_t len = strlen(table_->name_) + 1;
		char* table_name = mem_alloc(len);
		memcpy(table_name, table_->name_, len);
		new_con->con_type = JOIN;
		Pair* p = new_pair;
		PairSetFirst(p, table_name);
		new_con->left_opand = p;
		if (con_ == NULL)
			con_ = new_con;
		else {
			new_con->next_ = con_;
			con_ = new_con;
		}
	);
	return con_;
}
