#include"Sqlparse.h"
#include"../BaseStruct/Listhead.h"
#include"../StorageEngine/BufferManager.h"
//typedef struct select{
//	DBitems *select_items, *from_items, *group_by_items;
//	WhereNode* condition;
//	WhereNode* having;
//	JoinNode* join;
//}SelectNode;

SelectNode* new_select_node(){
	SelectNode* n = mem_alloc(sizeof(SelectNode));
	n->from_items = NULL;
	n->select_items = NULL;
	return n;
}

JoinNode* from_get_join(DBnode* db,DBitems* head) {
	DBitems* item;
	JoinNode *j_head = NULL;
	BufferManager* bm = get_buffman(db->id_);
	JoinNode* n;
	LIST_FOREACH(item, head,
		n = new_join_node(item->table_);
		n->page_ = buf_get_page(bm, TABLE_GET_NAME(n->table_), n->pid);
		if (j_head == NULL)
			j_head = n;
		else 
			j_head = join_lr(j_head, n);
		);
	return j_head;
}

int parse_select(char* errmsg,DBnode* db, Token** curr,QueryNode** qnode){

	*qnode = new_select_query();
	SelectNode* sel_node = (*qnode)->select_node;
	
	if (TOKEN_TYPE == STAR)
		NEXT_TOKEN;
	else if (TOKEN_TYPE == ID){
		if (get_item_list(errmsg, &sel_node->select_items,
			db, curr, TAB_COL_ITEM) == SQL_ERROR) 
			goto ERROR;
	}else 
		PARSE_ERROR("È±ÉÙ ID »ò *");

	if (TOKEN_TYPE != FROM) 
		PARSE_ERROR("È±ÉÙFROM");
	NEXT_TOKEN;

	if (get_item_list(errmsg, &sel_node->from_items, db, curr,FROM_ITEM) == SQL_ERROR)
		goto ERROR;
	if(sel_node->select_items && 
		check_item_list(errmsg, sel_node->select_items, sel_node->from_items) == SQL_ERROR)
		goto ERROR;

	sel_node->join = from_get_join(db, sel_node->from_items);

	if (TOKEN_TYPE == WHERE) {
		NEXT_TOKEN;
		if ((sel_node->condition = parse_where(errmsg, db, curr, sel_node->from_items)) == NULL)
			goto ERROR;
	}

	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

char* execute_from(DBnode* db, JoinNode* join) {
	if (join->is_join) {
		size_t left_len = TABLE_GET_DATA_LEN(join->left->table_),
			right_len = TABLE_GET_DATA_LEN(join->right->table_);
		char* join_row = mem_alloc(left_len + right_len);
		memcpy(join_row, execute_from(db, join->left), left_len);
		memcpy(join_row + left_len, 
			execute_from(db, join->right), right_len);

		return join_row;
	}else {
		if ((join->rid = get_row_index(join->page_, join->rid)) == P_ERROR) {
			BufferManager* bm = get_buffman(db->id_);
			join->pid++;
			join->rid = 0;
			if ((join->page_ = buf_get_page(bm, TABLE_GET_NAME(join->table_), join->pid)) == NULL)
				return SQL_ERROR;
		}
		return page_get_row(join->page_, join->rid++);
	}
}

int execute_select(char* errmsg,DBnode* db,SelectNode* sel) {
	char* row = execute_from(db, sel->join);
	WhereNode* con = sel->condition;
	//con->

}

int execute_where(char* row,int ref, DBnode* db, WhereNode* con) {
	if (con->operator_ == EQUAL ||
		con->operator_ == NOT_EQUAL ||
		con->operator_ == GREATERTHAN ||
		con->operator_ == GREATER_OR_EQ ||
		con->operator_ == LESSTHAN ||
		con->opand == LESS_OR_EQ)
	con->
}

int fliter_row(WhereNode* con, char* row) {
	
}

//__inline Relation* rela_filter_col_key(Relation* rela, Pair* schema, void* target,int opertype) {
//	struct rehead* col_ = rela_search_col(rela, schema);
//	Vector* data_set = &rela->data_set;
//	int offset_ = col_->rec_offset;
//	int data_type = col_->data_type;
//	int len_ = data_set->usedsize_;
//	void** rec_data = data_set->vector_;
//	for (int i = 0; i < len_; i++) {
//		obj* o = rec_data[i];
//		if (!o) continue;
//		unsigned char* data_ = o->data_;
//		if (filter_cmp(data_ + offset_, target, opertype, data_type))
//			continue;
//		if (o->ref_count == 1)
//			rec_data[i] = NULL;
//		dec_ref(o);
//	}
//	return rela;
//}

int __inline filter_cmp(void* value, void* target, int opertype,int datatype) {
	switch (opertype)
	{
	case EQUAL:
		switch (datatype)
		{
		case INT:
			return *(int*)value == *(int*)target;
		case FLOAT:
			return *(float*)value == *(float*)target;
		default:
			return strcmp(value, target) == 0 ? 1 : 0;
		}
	case LESSTHAN:
		switch (datatype)
		{
		case INT:
			return (*(int*)value < *(int*)target) ? 1 : 0;
		case FLOAT:
			return (*(float*)value < *(float*)target) ? 1 : 0;
		default:
			return strcmp(value, target) < 0 ? 1 : 0;
		}
	case GREATERTHAN:
		switch (datatype)
		{
		case INT:
			return (*(int*)value > *(int*)target) ? 1 : 0;
		case FLOAT:
			return (*(float*)value > *(float*)target) ? 1 : 0;
		default:
			return strcmp(value, target) > 0 ? 1 : 0;
		}
	case NOT_EQUAL:
		return filter_cmp(value, target, EQUAL, datatype) ? 0 : 1;
		//todo
	default:
		break;
	}
	return -1;
}
