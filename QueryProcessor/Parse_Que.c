#include "Sqlparse.h"
#include "Parse_Que.h"
#include"../Mem/MemPool.h"
#include"../Mem/obj.h"
#include"Relation.h"

#define QUERY_ERROR(buf,var) do{\
fprintf(stderr, buf,var); goto ERROR;\
}while(0)

__inline void* create_con(int oper, void* leftcon, void* rightcon);
__inline void con_del(struct con* con);
__inline void* get_con_exp(DBnode* db, Token** curr);
__inline void* get_term(DBnode* db,Token** curr);
__inline void* get_factor(DBnode* db,Token** curr);
__inline int32_t get_base_exp(DBnode* db, Pair* p, Token** curr);
__inline int32_t check_con(DBnode* db, struct con* con,Vector* vfrom);
__inline int32_t check_schema(DBnode* db,Pair* table_col,Vector* vfrom); 
__inline void* create_join(Table* table, Vector* from);

__inline Relation* con_join_exec(Relation* rela, DBnode* db, struct con* con);
__inline Relation* con_where_exec(Relation*, DBnode* db, struct con* con);

int32_t check_schema(DBnode* db,Pair* p,Vector* vfrom){
	if (p == NULL || PairGetSecond(p) == NULL)
		return 0;
	Table* from_table;
	char* table_name = PairGetFirst(p);
	int hit_ = 0;
	if (table_name == NULL) {
		char* col_name = PairGetSecond(p);
		VECTOR_FOREACH(from_table, vfrom,
			if (DBNODE_SEARCH(from_table->col_head, col_name) != NULL)
				hit_++;
			);
		if (hit_ == 0) QUERY_ERROR("列名 %s 无效\n", col_name);
		if (hit_ > 1) QUERY_ERROR("列名 %s 不明确\n", col_name);
	}else{
		VECTOR_FOREACH(from_table, vfrom,
			if (strcmp(table_name, from_table->name_) == 0)
				hit_ = 1;
			);
		if (hit_ == 0) 
			QUERY_ERROR("表名 %s 无效\n", table_name);	
	}
	return 0;
ERROR:
	return -1;
}

int32_t check_con(DBnode * db,struct con * con,Vector* vfrom){
	if (con == NULL)
		return 0;
	if (check_con(db, con->left_con, vfrom) == -1)
		return -1;
	if (check_con(db, con->right_con, vfrom) == -1)
		return -1;
	if (check_schema(db, con->left_opand, vfrom) == -1)
		return -1;
	if (check_schema(db, con->right_opand, vfrom) == -1)
		return -1;
	return 0;
}

static __inline int32_t get_base_exp(DBnode* db,Pair* p, Token** curr) {
	if (TOKEN_TYPE == ID) {
		if (get_schema(db, p, curr) == -1)
			goto ERROR;
	}else if (TOKEN_TYPE == INT  ||
			  TOKEN_TYPE == TIME ||
			  TOKEN_TYPE == TEXT ||
			  TOKEN_TYPE == REAL ||
			  TOKEN_TYPE == CHAR ) {
		move_value(&(*curr)->value_, &PairGetFirst(p));
		NEXT_TOKEN;
	}else PARSE_ERROR("缺少 ID 或 INT 或 TIME 或 TEXT 或 REAL");
	return 0;
ERROR:
	return -1;
}

static __inline int get_schema(DBnode* db,Pair* sch,Token** curr) {
	if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
	if (NEXT_TOKEN_TYPE == DOT) {
		Table* target_table = NULL;
		if ((target_table = DBNODE_SEARCH(db->table_head, (*curr)->value_)) == NULL) PARSE_ERROR("表名无效");
		move_value(&(*curr)->value_, &sch->first_);
		NEXT_TOKEN;
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
		if (DBNODE_SEARCH(target_table->col_head, (*curr)->value_) == NULL) PARSE_ERROR("列名无效");
		move_value(&(*curr)->value_, &sch->second_);
	}else move_value(&(*curr)->value_, &sch->second_);
	NEXT_TOKEN;
	return 0;
ERROR:
	return -1;
}

static void* get_factor(DBnode* db,Token** curr) {
	struct con* con_ = NULL;
	if (TOKEN_TYPE == LB) {
		NEXT_TOKEN;
		if ((con_ = get_con_exp(db, curr)) == NULL) goto ERROR;
		if (TOKEN_TYPE != RB) PARSE_ERROR("缺少 )");
		NEXT_TOKEN;
		return con_;
	}
	con_ = mem_calloc(1,sizeof(struct con));
	con_->con_type = WHERE;
	Pair *opand_ = new_pair;
	//con_->left_opand = mem_calloc(1,sizeof(struct opand));
	if (get_base_exp(db, opand_, curr) == -1) goto ERROR;
	con_->left_opand = opand_;
	if (TOKEN_TYPE != LIKE  &&
		TOKEN_TYPE != EQUAL &&
		TOKEN_TYPE != LESSTHAN &&
		TOKEN_TYPE != GREATERTHAN &&
		TOKEN_TYPE != LESS_OR_EQ &&
		TOKEN_TYPE != GREATER_OR_EQ &&
		TOKEN_TYPE != EXISTS &&
		TOKEN_TYPE != NOT_EQUAL)
		PARSE_ERROR("缺少运算符");
	con_->operator_ = TOKEN_TYPE;
	NEXT_TOKEN;
	opand_ = new_pair;
	if (get_base_exp(db, opand_, curr) == -1) goto ERROR;
	con_->right_opand = opand_;
	return con_;
ERROR:
	con_del(con_);
	return NULL;		
}

static void* get_term(DBnode* db,Token** curr) {
	struct con* con_ = NULL;
	if ((con_ = get_factor(db,curr)) == NULL)
		goto ERROR;
	for (;;) {
		if (TOKEN_TYPE != AND) break;
		struct con* right_con;
		NEXT_TOKEN;
		if ((right_con = get_factor(db,curr)) == NULL) goto ERROR;
		con_ = create_con(AND, con_, right_con);
		con_->con_type = WHERE;
		con_->operator_ = AND;
	}
	return con_;
ERROR:
	con_del(con_);
	return NULL;
}

void* get_con_exp(DBnode* db, Token** curr) {
	struct con* con_ = NULL;
	if ((con_ = get_term(db,curr)) == NULL)
		goto ERROR;
	for (;;) {
		if (TOKEN_TYPE != OR) break;
		struct con* right_con = NULL;
		NEXT_TOKEN;
		if ((right_con = get_term(db,curr)) == NULL) goto ERROR;
		con_ = create_con(OR, con_, right_con);
		con_->con_type = WHERE;
		con_->operator_ = OR;
	}
	return con_;
ERROR:
	con_del(con_);
	return NULL;
}


Relation* con_join_exec(Relation* rela, DBnode* db, struct con* con) {
	void* next_ = con->next_;
	Pair* left_opand = con->left_opand;
	char * s = PairGetFirst(left_opand);
	Table* t_ = DBNODE_SEARCH(db->table_head, PairGetFirst(left_opand));
	if (con->operator_ == NULL && next_ == NULL)//todo
		return get_relation(t_);
	else 
		return rela_loop_join(con_join_exec(rela, db, next_), get_relation(t_));
}

Relation* con_where_exec(Relation* rela, DBnode* db, struct con* con) {
	struct con* left_con = con->left_con;
	struct con* right_con = con->right_con;
	struct con* next_ = con->next_;
	Pair* left_opand = con->left_opand;
	Pair* right_opand = con->right_opand;	
	Relation *rela_ = NULL;
	if (next_)
		rela_ = con_join_exec(rela, db, next_);
	else
		rela_ = rela;
	int operator_ = con->operator_;
	Relation* return_res = NULL;
	switch (operator_)
	{
	case OR: {
		Relation* temp_ = rela_create(rela_->data_len, 0, rela_->rehead_list, rela_->data_set.usedsize_);
		obj* obj_ = NULL;
		int ref_count = 0;
		VECTOR_FOREACH(obj_, &rela_->data_set,
			if (obj_) {
				ref_count = obj_->ref_count + 1;
				break;
			}
		);
		VECTOR_FOREACH(obj_, &rela_->data_set,
			if(obj_)
				inc_ref(obj_);
		);
		con_execute(rela_, db, left_con);
		VECTOR_FOREACH(obj_, &rela_->data_set,
			if (obj_) {
				if (obj_->ref_count < ref_count) {
					inc_ref(obj_);
					VECTOR_PUSHBACK(&temp_->data_set, obj_);
				}
			}
		);
		con_execute(temp_, db, right_con);
		Vector* v_ = &rela_->data_set;
		int len_ = VectorGetUsedSize(v_);
		void** data_v = v_->vector_;
		for (int i = 0; i < len_; i++) {
			obj* o = data_v[i];
			if (o) {
				if (o->ref_count == 1)
					data_v[i] = NULL;
				dec_ref(o);
			}
		}
		rela_clear_v(temp_);
		mem_free(temp_);
		return_res = rela_;
		break;
	}
	case AND: {
		Relation* temp_ = rela_create(rela_->data_len, 0, rela_->rehead_list, rela_->data_set.usedsize_);
		obj* obj_ = NULL;
		int ref_count = 0;
		VECTOR_FOREACH(obj_, &rela_->data_set,
			if (obj_) {
				ref_count = obj_->ref_count + 1;
				break;
			}
		);
		VECTOR_FOREACH(obj_, &rela_->data_set,
			if(obj_)
				inc_ref(obj_);
		);
		con_execute(rela_, db, left_con);
		VECTOR_FOREACH(obj_, &rela_->data_set,
			if (obj_) {
				if (obj_->ref_count == ref_count) {
					VECTOR_PUSHBACK(&temp_->data_set, obj_);
				}
			}
		);
		con_execute(temp_, db, right_con);
		Vector* v_ = &rela_->data_set;
		int len_ = VectorGetUsedSize(v_);
		void** data_v = v_->vector_;
		for (int i = 0; i < len_; i++) {
			obj* o = data_v[i];
			if (o) {
				if (o->ref_count == 1)
					data_v[i] = NULL;
				dec_ref(o);
			}
		}
		rela_clear_v(temp_);
		mem_free(temp_);
		return_res = rela_;
		break;
	}
	default:
		return_res = rela_filter(rela_, db, left_opand, right_opand, operator_);
		break;
	}
	return return_res;
}

//Relation* con_execute(Relation* rela, DBnode* db, Logicplan* con) {
//	switch (con->con_type)
//	{
//	case JOIN:
//		return con_join_exec(rela, db, con);
//	case WHERE:
//		return con_where_exec(rela,db, con);
//	case SELECT:
//		return;
//	default:
//		break;
//	}
//}

int parse_select(Arena* arena,DBnode* db,Token** curr) {
	Vector v_col;
	Vector v_from;
	VECTOR_INIT(&v_col, 4);
	VECTOR_INIT(&v_from, 4);
	struct con* con_ = NULL;
	if (TOKEN_TYPE == STAR)
		NEXT_TOKEN;
	else if (TOKEN_TYPE == ID) {
		for (;;) {
			Pair *schema_name = new_pair;
			if (get_schema(db, schema_name, curr) == -1) goto ERROR;
			VECTOR_PUSHBACK(&v_col, schema_name);
			if (TOKEN_TYPE != COMMA) break;
			NEXT_TOKEN;
		}
	}else PARSE_ERROR("缺少 ID 或 *");
	if (TOKEN_TYPE != FROM) PARSE_ERROR("缺少FROM");
	NEXT_TOKEN;
	for (;;) {
		if (TOKEN_TYPE != ID) PARSE_ERROR("缺少 ID ");
		Table* table_;
		if ((table_ = DBNODE_SEARCH(db->table_head, (*curr)->value_)) == NULL)
			PARSE_ERROR(" FROM 语句 表名无效");
		VECTOR_PUSHBACK(&v_from, table_);
		//无法判断sql语句结束
		if (MOVE_NEXT_TOKEN_TYPE != COMMA) break;
		NEXT_TOKEN;
	}

	Pair *schema_;
	VECTOR_FOREACH(schema_, &v_col,
		if (check_schema(db, schema_, &v_from) == -1)
			goto ERROR;
		);

	struct con* join_con = create_join(db->table_head, &v_from);
	if (TOKEN_TYPE == WHERE) {
		NEXT_TOKEN;
		if (((con_ = get_con_exp(db, curr)) == NULL || check_con(db, con_, &v_from) == -1))
			goto ERROR;
		con_->next_ = join_con;
	}else con_ = join_con;


	VECTOR_FOREACH(schema_, &v_col, PAIR_FREE(schema_););
	VECTOR_CLEAR_NODE(&v_col);
	VECTOR_CLEAR_NODE(&v_from);
	con_del(con_);

	return 0;
ERROR:
	VECTOR_FOREACH(schema_, &v_col, PAIR_FREE(schema_););
	VECTOR_CLEAR_NODE(&v_col);
	VECTOR_CLEAR_NODE(&v_from);
	con_del(con_);
	return -1;
}

void* create_join(Table* table, Vector* from) {
	Table* table_;
	struct con* con_ = NULL;	
	VECTOR_FOREACH(table_, from, 	
		struct con* new_con = mem_calloc(1, sizeof(*new_con));
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

void* create_con(int oper, void* leftcon, void* rightcon){
	struct con* con_ = mem_calloc(1,sizeof(struct con));
	con_->operator_ = oper;
	con_->left_con = leftcon;
	con_->right_con = rightcon;
	return con_;
}

void con_del(struct con* con){
	if (con == NULL) return;
	con_del(con->left_con);
	con_del(con->right_con);
	con_del(con->next_);
	PAIR_FREE(con->left_opand);
	PAIR_FREE(con->right_opand);
	mem_free(con);
}




