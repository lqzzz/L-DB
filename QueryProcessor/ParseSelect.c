#include"Sqlparse.h"
#include"../BaseStruct/Listhead.h"

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

JoinNode* from_get_join(DBitems* head) {
	DBitems* item;
	JoinNode *j_head = NULL;
	LIST_FOREACH(item, head,
		JoinNode* n = new_join_node(item->table_);
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

	sel_node->join = from_get_join(sel_node->from_items);

	if (TOKEN_TYPE == WHERE) {
		NEXT_TOKEN;
		if ((sel_node->condition = parse_where(errmsg, db, curr, sel_node->from_items)) == NULL)
			goto ERROR;
	}

	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

int execute_select(char* errmsg,DBnode* db,SelectNode* sel) {
	SelectNode* node = sel;
}