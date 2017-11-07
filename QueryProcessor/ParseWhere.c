#include"Sqlparse.h"

static WhereNode* get_exp(char* errmsg,DBnode* db, Token** curr,DBitems* itab);
static WhereNode* get_term(char* errmsg,DBnode* db,Token** curr,DBitems* itab);
static WhereNode* get_factor(char* errmsg,DBnode* db,Token** curr,DBitems* itab);
static WhereNode* get_plus_sub(char* errmsg,DBnode* db,Token** curr,DBitems* itab);
static WhereNode* get_mul_div(char* errmsg,DBnode* db,Token** curr,DBitems* itab);
static __inline int32_t get_base_exp(char* errmsg,DBnode* db, Pair* p, Token** curr);

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
	free_dbitem(n->left_opand);
	free_dbitem(n->right_opand);
	free_where(n);
}

WhereNode* get_exp(char* errmsg,DBnode* db, Token** curr,DBitems* itab) {
	WhereNode* exp = NULL;
	if ((exp = get_term(errmsg,db,curr, itab)) == NULL)
		goto ERROR;
	for (;;) {
		if (TOKEN_TYPE != OR) 
			break;
		WhereNode* rn = NULL;
		//struct con* right_con = NULL;
		NEXT_TOKEN;
		if ((rn = get_term(errmsg,db, curr, itab)) == NULL)
			goto ERROR;
		WhereNode* or = new_where_node(OR);
		or->right = rn;
		or->left = exp;
		exp = or;
	}
	return exp;
ERROR:
	free_where(exp);
	return NULL;
}

WhereNode* get_term(char* errmsg,DBnode* db,Token** curr,DBitems* itab){
	WhereNode* exp = NULL;
	if ((exp = get_factor(errmsg,db,curr, itab)) == NULL)
		goto ERROR;
	for (;;) {
		if (TOKEN_TYPE != AND) 
			break;
		WhereNode* rn;
		NEXT_TOKEN;
		if ((rn = get_term(errmsg,db, curr, itab)) == NULL)
			goto ERROR;
		WhereNode* and = new_where_node(AND);
		and->right = rn;
		and->left = exp;
		exp = and;
	}
	return exp;
ERROR:
	free_where(exp);
	return NULL;
}

WhereNode* get_factor(char* errmsg,DBnode* db, Token** curr,DBitems* itab) {
	WhereNode* exp = NULL;
	if ((exp = get_plus_sub(errmsg, db, curr, itab)) == NULL)
		goto ERROR;
	for (;;) {
		if (TOKEN_TYPE != IN    &&
			TOKEN_TYPE != LIKE  &&
			TOKEN_TYPE != EQUAL &&
			TOKEN_TYPE != LESSTHAN &&
			TOKEN_TYPE != GREATERTHAN &&
			TOKEN_TYPE != LESS_OR_EQ &&
			TOKEN_TYPE != GREATER_OR_EQ &&
			TOKEN_TYPE != EXISTS &&
			TOKEN_TYPE != NOT_EQUAL)
			break;
		int op = TOKEN_TYPE;
		WhereNode* rn;
		NEXT_TOKEN;
		if ((rn = get_factor(errmsg,db, curr, itab)) == NULL)
			goto ERROR;
		WhereNode* f = new_where_node(op);
		f->right = rn;
		f->left = exp;
		exp = f;
	}
	return exp;
ERROR:
	free_where(exp);
	return NULL;
}

WhereNode* get_plus_sub(char* errmsg,DBnode* db, Token** curr,DBitems* itab) {
	WhereNode* exp;
	if ((exp = get_mul_div(errmsg,db,curr, itab)) == NULL)
		goto ERROR;
	for (;;) {
		if (TOKEN_TYPE != PLUS && TOKEN_TYPE != SUB)
			break;
		int op = TOKEN_TYPE;
		WhereNode* rn;
		NEXT_TOKEN;
		if ((rn = get_plus_sub(errmsg,db, curr, itab)) == NULL)
			goto ERROR;
		WhereNode* f = new_where_node(op);
		f->right = rn;
		f->left = exp;
		exp = f;
	}
	return exp;
ERROR:
	free_where(exp);
	return NULL;
}

WhereNode* get_mul_div(char* errmsg,DBnode* db, Token** curr,DBitems* itab) {
	WhereNode* exp = NULL;
	if (TOKEN_TYPE == LB) {
		NEXT_TOKEN;
		if ((exp = get_plus_sub(errmsg, db, curr, itab)) == NULL)
			goto ERROR;
		if (TOKEN_TYPE != RB) 
			PARSE_ERROR("È±ÉÙ )");
		NEXT_TOKEN;
		return exp;
	}
	for (;;) {
		DBitems *litem;
		if (TOKEN_TYPE == ID && (litem = get_item(errmsg, db, curr, TAB_COL_ITEM)) == NULL)
			goto ERROR;
		if ((litem = get_item(errmsg, db, curr, BASE_ITEM)) == NULL)
			goto ERROR;
		if (check_item(errmsg, litem, itab) == SQL_ERROR)
			goto ERROR;

		exp = new_where_node(NULL);
		exp->left_opand = litem;

		if (TOKEN_TYPE != MUL && TOKEN_TYPE != DIV)
			break;

		int op = TOKEN_TYPE;
		NEXT_TOKEN;
		WhereNode* r;
		if ((r = get_mul_div(errmsg, db, curr, itab)) == NULL)
			goto ERROR;
		//check type
		WhereNode* n = new_where_node(op);
		n->left = exp;
		n->right = r;
		exp = n;
	}
	return exp;
ERROR:
	free_where(exp);
	return NULL;
}
