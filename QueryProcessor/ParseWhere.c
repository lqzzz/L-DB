#include"Sqlparse.h"
#define BOOL 0
#define NUMBER 1

//static enum Type{
//	logic,
//	math,
//	
//};

WhereNode* parse_where(char* errmsg,DBnode* db, Token** curr,DBitems* itab);
static WhereNode* get_term(char* errmsg, DBnode* db, Token** curr, DBitems* itab);
static WhereNode* get_factor(char* errmsg,DBnode* db,Token** curr,DBitems* itab);
static WhereNode* get_plus_sub(char* errmsg,DBnode* db,Token** curr,DBitems* itab);
static WhereNode* get_mul_div(char* errmsg,DBnode* db,Token** curr,DBitems* itab);
static WhereNode* get_base_item(char* errmsg, DBnode* db, Token** curr, DBitems* itab);

//int execute_where(char* errmsg, DBnode* db, WhereNode* condition) {
//	WhereNode* node = condition;
//	node->res_type	
//}

WhereNode* new_where_node(int op) {
	WhereNode* where_node = mem_calloc(1,sizeof(WhereNode));
	where_node->operator_ = op;
	return where_node;
}

void free_where(WhereNode* n) {
	if (n == NULL)
		return;
	int operator_ = n->operator_;
	free_where(n->left);
	free_where(n->right);
	free_dbitem(n->opand);
	mem_free(n);
}

WhereNode* parse_where(char* errmsg,DBnode* db, Token** curr,DBitems* itab) {
	WhereNode* exp = NULL,*rn = NULL;
	if ((exp = get_term(errmsg,db,curr, itab)) == NULL)
		goto ERROR;
	while(TOKEN_TYPE == OR){
		NEXT_TOKEN;
		WhereNode* rn;
		if ((rn = parse_where(errmsg, db, curr, itab)) == NULL && rn->res_type != BOOL)
			goto ERROR;
		WhereNode* or = new_where_node(OR);
		or->right = rn;
		or->left = exp;
		or ->res_type = BOOL;
		exp = or;
	}
	return exp;
ERROR:
	free_where(exp);
	return NULL;
}

WhereNode* get_term(char* errmsg,DBnode* db,Token** curr,DBitems* itab){
	WhereNode* exp = NULL,*rn = NULL;
	if ((exp = get_factor(errmsg,db,curr, itab)) == NULL)
		goto ERROR;
	while(TOKEN_TYPE == AND) {
		NEXT_TOKEN;
		if ((rn = get_term(errmsg, db, curr, itab)) == NULL && rn->res_type != BOOL)
			goto ERROR;
		WhereNode* and = new_where_node(AND);
		and->right = rn;
		and->left = exp;
		and->res_type = BOOL;
		exp = and;
	}
	return exp;
ERROR:
	free_where(exp);
	free_where(rn);
	return NULL;
}

WhereNode* get_factor(char* errmsg, DBnode* db, Token** curr, DBitems* itab) {
	WhereNode *exp = NULL, *right_ = NULL;
	if ((exp = get_plus_sub(errmsg, db, curr, itab)) == NULL)
		goto ERROR;
	while (TOKEN_TYPE == IN ||
		TOKEN_TYPE == LIKE  ||
		TOKEN_TYPE == EQUAL ||
		TOKEN_TYPE == LESSTHAN ||
		TOKEN_TYPE == GREATERTHAN ||
		TOKEN_TYPE == LESS_OR_EQ ||
		TOKEN_TYPE == GREATER_OR_EQ ||
		TOKEN_TYPE == EXISTS ||
		TOKEN_TYPE == NOT_EQUAL) {

		int op = TOKEN_TYPE;
		NEXT_TOKEN;

		if ((right_ = get_factor(errmsg, db, curr, itab)) == NULL)
			goto ERROR;
		if ((op == EQUAL ||
			op == LESSTHAN ||
			op == GREATERTHAN ||
			op == LESS_OR_EQ ||
			op == GREATER_OR_EQ ||
			op == NOT_EQUAL) &&
			(exp->res_type != NUMBER || right_->res_type != NUMBER))
			PARSE_ERROR("");
		
		WhereNode *f = new_where_node(op);
		f->right = right_;
		f->left = exp;
		f->res_type = BOOL;
		exp = f;
	}
	return exp;

ERROR:
	free_where(exp);
	free_where(right_);
	return NULL;
}

WhereNode* get_plus_sub(char* errmsg,DBnode* db, Token** curr,DBitems* itab) {
	WhereNode *exp = NULL, *rn = NULL;
	if ((exp = get_mul_div(errmsg,db,curr, itab)) == NULL)
		goto ERROR;
	for (;;) {
		if (TOKEN_TYPE != PLUS && TOKEN_TYPE != SUB)
			break;
		int op = TOKEN_TYPE;
		NEXT_TOKEN;
		if ((rn = get_plus_sub(errmsg, db, curr, itab)) == NULL && rn->res_type != NUMBER)
			goto ERROR;
		WhereNode* f = new_where_node(op);
		f->right = rn;
		f->left = exp;
		f->res_type = NUMBER;
		exp = f;
	}
	return exp;
ERROR:
	free_where(exp);
	free_where(rn);
	return NULL;
}

WhereNode* get_base_item(char* errmsg, DBnode* db, Token** curr, DBitems* itab) {
	WhereNode *exp = NULL;
	if (TOKEN_TYPE == LB) {
		NEXT_TOKEN;
		if ((exp = parse_where(errmsg, db, curr, itab)) == NULL)
			goto ERROR;
		if (TOKEN_TYPE != RB)
			PARSE_ERROR("È±ÉÙ )");
		NEXT_TOKEN;
	}else {
		DBitems *item;
		int token_type = TOKEN_TYPE;

		if (token_type == ID && (item = get_item(errmsg, db, curr, TAB_COL_ITEM)) == NULL)
			goto ERROR;
		else if (token_type != ID && (item = get_item(errmsg, db, curr, BASE_ITEM)) == NULL)
			goto ERROR;
		if (check_item_list(errmsg, item, itab) == SQL_ERROR)
			goto ERROR;
		
		exp = new_where_node(NULL);
		exp->opand = item;
		exp->res_type = item->col_ ? item->col_->column_data_type :
			item->base_item->token_type;
		if (exp->res_type == INT || exp->res_type == FLOAT)
			exp->res_type = NUMBER;
	}
	return exp;
ERROR:
	free_where(exp);
	return NULL;
}

WhereNode* get_mul_div(char* errmsg, DBnode* db, Token** curr, DBitems* itab) {
	WhereNode *exp = NULL, *r = NULL;
	if ((exp = get_base_item(errmsg,db,curr, itab)) == NULL)
		goto ERROR;
	while(TOKEN_TYPE == MUL || TOKEN_TYPE == DIV) {
		int op = TOKEN_TYPE;

		NEXT_TOKEN;
		if ((r = get_mul_div(errmsg, db, curr, itab)) == NULL && r->res_type != NUMBER)
			goto ERROR;
		WhereNode* n = new_where_node(op);
		n->left = exp;
		n->right = r;
		n->res_type = NUMBER;
		exp = n;
	}
	return exp;
ERROR:
	free_where(exp);
	free_where(r);
	return NULL;
}
