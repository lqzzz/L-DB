#include"Relation.h"
#include"../Mem/obj.h"

#define RELA_CLEAR_HEAD(rela)do{\
	struct rehead* node_ = NULL;\
	LIST_DEL_ALL(node_,&rela->rehead_list->head_,mem_free(node_););\
}while(0)

#define RELA_CLEAR_V(rela) VECTOR_CLEAR_NODE(&rela->data_set)
#define RELA_DEL_V(rela) VECTOR_CLEAR_VAL(&rela->data_set) 
#define RELA_DEL(rela)do{\
	RELA_CLEAR_HEAD(rela)\
	RELA_CLEAR_V(rela);\
	mem_free(rela);\
}while(0)
#define RELA_FREE(rela) do{\
	if(rela->is_join){\
		VECTOR_CLEAR_VAL(&rela->data_set);\
		RELA_DEL(rela);\
	}else RELA_DEL(rela);\
}while(0)

__inline Relation* rela_filter_col_col(Relation*, Pair*, Pair*, int);
__inline Relation* rela_filter_col_key(Relation*, Pair*, void*, int);
__inline int filter_cmp(void* value, void* target, int opertype, int datatype);

Relation* new_relation(Table * table){
	Relation* rela_ = mem_alloc(sizeof(*rela_));
	rela_->data_len = table->data_len;
	rela_->is_join = 0;
	rela_->row_ = mem_calloc(1, table->data_len);
	rela_->file_name = table->name_;
	VECTOR_INIT(&rela_->data_set, INIT_LEN);
	return rela_;
}

Relation* rela_create(size_t datalen, int isjoin,struct rehead* head,int v_size) {
	Relation* rela_ = mem_alloc(sizeof(*rela_));
	rela_->data_len = datalen;
	rela_->is_join = isjoin;
	rela_->rehead_list = head;
	VECTOR_INIT(&rela_->data_set, v_size);
	return rela_;
}

void rela_add_col(Relation* rela, Column* col) {
	struct rehead* re_new = mem_calloc(1, sizeof(*re_new));
	struct rehead* headlist = rela->rehead_list;
	LIST_INIT(&re_new->head_);
	re_new->data_type = col->data_type;
	re_new->rec_offset = col->rec_offset;
	PairSetFirst(&re_new->schema_name, col->table_name);
	PairSetSecond(&re_new->schema_name, col->name_);
	if (headlist == NULL) headlist = re_new;
	else LIST_ADD_TAIL(&headlist->head_, &re_new->head_);
}

Relation* rela_loop_join(Relation * left, Relation* right) {
	struct rehead* copy_left = left->rehead_list;
	struct rehead* copy_right = right->rehead_list;
	struct rehead* node_ = NULL;
	LIST_FOREACH(node_, copy_right,
		node_->rec_offset += left->data_len;
	);
	LIST_JOIN(copy_left, copy_right);
	struct rehead* copy = NULL;
	int i = 0;
	Vector* out_v = &left->data_set;
	Vector* inter_v = &right->data_set;
	size_t left_data_len = left->data_len;
	size_t right_data_len = right->data_len;
	size_t data_len = left_data_len + right_data_len;
	Relation* join_rela = 
		rela_create(data_len, 1, copy_left, 
			VectorGetUsedSize(out_v) * VectorGetUsedSize(inter_v));
	Vector* join_v = &join_rela->data_set;
	obj *out_var, *inter_var;
	VECTOR_FOREACH(out_var, out_v,
		VECTOR_FOREACH(inter_var, inter_v,
			void* join_data = mem_alloc(data_len);
			memcpy(join_data, out_var->data_, 
				left_data_len);
			memcpy((char*)join_data + left_data_len, 
				inter_var->data_, right_data_len);
			obj* o = new_obj(join_data);
			inc_ref(o);
			VECTOR_PUSHBACK(join_v, o);
		);
		free_obj(out_var);
	);
	VECTOR_FOREACH(inter_var, inter_v,
		free_obj(inter_var);
	);
	RELA_CLEAR_V(left);
	RELA_CLEAR_V(right);
	mem_free(left);
	mem_free(right);
	return join_rela;
}

struct rehead* rela_search_col(Relation* rela, Pair* schema){
	struct rehead* head_ = rela->rehead_list;
	struct rehead* node_ = NULL;
	char* target_table_name = PairGetFirst(schema);
	char* target_cols_name = PairGetSecond(schema);

	LIST_FOREACH(node_, head_,
		char* table_name = PairGetFirst(&node_->schema_name);
		char* col_name = PairGetSecond(&node_->schema_name);
		if (target_table_name == NULL) {
			if (strcmp(target_cols_name, col_name) == 0)
				return node_;
		}else if (strcmp(target_cols_name, col_name) == 0 &&
				  strcmp(target_table_name, table_name) == 0){
				return node_;
		}
		//return -1;
		);
	return NULL;
}

Relation* get_relation(Table* table){
	struct rehead* headlist = NULL;
	Column* col_;
	char* table_name = table->name_;
	LIST_FOREACH(col_, table->col_head,
		struct rehead* re_new = mem_calloc(1, sizeof(*re_new));
		LIST_INIT(&re_new->head_);
		re_new->data_type = col_->data_type;
		re_new->rec_offset = col_->rec_offset;
		re_new->max_len = &col_->max_data_len;
		PairSetFirst(&re_new->schema_name, table_name);
		PairSetSecond(&re_new->schema_name, col_->name_);
		if (headlist == NULL) headlist = re_new;
		else LIST_ADD_TAIL(&headlist->head_, &re_new->head_);
		);

	int rec_count = table->rec_size;
	Relation *rela_ = rela_create(table->data_len, 0, headlist, rec_count);
	return rela_;
}

size_t rela_get_max_len(Relation * rela, int colindex) {
	struct rehead* head_ = rela->rehead_list;
	struct rehead* node_;
	int index_ = 0;
	LIST_FOREACH(node_, head_,
		if (index_++ == colindex)
			break;
		);
	size_t max_len = strlen(PairGetSecond(&node_->schema_name));
	obj* obj_;
	unsigned char  *data_node;
	switch (node_->data_type)
	{
	case INT: {
		int int_len = 0;
		obj* obj_;
		VECTOR_FOREACH(obj_, &rela->data_set,
			if (obj_) {
				data_node = obj_->data_;
				int num_ = *(int*)((char*)data_node + node_->rec_offset);
				while (num_ != 0) {
					num_ = num_ / 10;
					int_len++;
				}
				if (int_len > max_len)
					max_len = int_len;
				int_len = 0;
			}
			);
		break;
	}
	case REAL: {
		int int_len = 0;
		VECTOR_FOREACH(obj_, &rela->data_set,
			if (obj_) {
				data_node = obj_->data_;
				int num_ = *(int*)((char*)data_node + node_->rec_offset);
				while (num_ != 0) {
					num_ = num_ / 10;
					int_len++;
			}
		}
		int_len += 2;
		if (int_len > max_len)
			max_len = int_len;
		int_len = 0;
		);
		break;
	}
	case CHAR:
		VECTOR_FOREACH(obj_, &rela->data_set,
			if (obj_) {
				data_node = obj_->data_;
				size_t str_len = strlen((char*)data_node + node_->rec_offset);
				if (max_len < str_len)
					max_len = str_len;
			}
			);
		break;
	default:
		break;
	}
	node_->max_len = max_len;
	return max_len;
}

void rela_print(Relation * rela){
	struct rehead* node_ = NULL;
	struct rehead* head_ = rela->rehead_list;
	int index_ = 0;
	LIST_FOREACH(node_, head_,
		int max_len = rela_get_max_len(rela, index_++);
		printf("+");
		for (int i = 0; i < max_len; i++)
			printf("-");
		);
	printf("+\n");
	LIST_FOREACH(node_, head_,
		int max_len = node_->max_len;
		char* str_ = PairGetSecond(&node_->schema_name);
		printf("|%-*s", max_len, str_);
		);
	printf("|\n");
	LIST_FOREACH(node_, head_,
		int max_len = node_->max_len;
		printf("+");
		for (int i = 0; i < max_len; i++)
			printf("-");
		);
	printf("+\n");
	obj* obj_;
	VECTOR_FOREACH(obj_, &rela->data_set,
		if (obj_) {
			unsigned char* item_ = obj_->data_;
			struct rehead* node_ = NULL;
			LIST_FOREACH(node_, rela->rehead_list,
				switch (node_->data_type)
				{
				case INT:
					printf("|%-*d", node_->max_len, *(int*)(item_ + node_->rec_offset));
					break;
				case REAL:
					printf("|%-*d", node_->max_len, *(int*)(item_ + node_->rec_offset));
					break;
				case CHAR:
					printf("|%-*s", node_->max_len, item_ + node_->rec_offset);
					break;
				default:
					break;
				}
			);
			dec_ref(obj_);
			printf("|\n");
		}
		);
	printf("\n");
	RELA_CLEAR_HEAD(rela);
	RELA_CLEAR_V(rela);
	mem_free(rela);
}

Relation* rela_or(Relation* left, Relation* right){
	Vector* left_data_set = &left->data_set;
	Vector* right_data_set = &right->data_set;
	Relation* new_rela = rela_create(left->data_len, left->is_join,left->rehead_list,VectorGetUsedSize(left_data_set) + VectorGetUsedSize(right_data_set));
	Vector* new_data_set = &new_rela->data_set;
	void* value_;
	VECTOR_FOREACH(value_, left_data_set,
		VECTOR_PUSHBACK(new_data_set, value_);
		);
	VECTOR_FOREACH(value_, right_data_set,
		VECTOR_PUSHBACK(new_data_set, value_);
		);
	RELA_CLEAR_V(left);
	RELA_CLEAR_V(right);
	mem_free(left);
	mem_free(right);
	return new_rela;
}

Relation* rela_filter(Relation* rela,DBnode* db, Pair* l, Pair* r,int oper_type) {
	char* l_s = PairGetSecond(l);
	char* r_s = PairGetSecond(r);
	if (l_s && r_s)
		return rela_filter_col_col(rela, l, r, oper_type);
	else if (r_s == NULL)
		return rela_filter_col_key(rela, l, PairGetFirst(r), oper_type);
	else 
		return rela_filter_col_key(rela, r, PairGetFirst(l), oper_type);
}

__inline Relation* rela_filter_col_col(Relation* rela, Pair* first, Pair* second, int opertype) {
	struct rehead* col_f = rela_search_col(rela, first);
	struct rehead* col_s = rela_search_col(rela, second);
	Vector* data_set = &rela->data_set;
	int data_type = col_f->data_type;
	int offset_f = col_f->rec_offset;
	int offset_s = col_s->rec_offset;
	int is_join = rela->is_join;
	int len_ = data_set->usedsize_;
	obj** rec_data = data_set->vector_;
	for (int i = 0; i < len_; i++) {
		obj* o = rec_data[i];
		if (!o) continue;
		unsigned char* data_ = o->data_;
		if (filter_cmp(data_ + offset_f, data_ + offset_s, opertype, data_type))
			continue;
		if (o->ref_count == 1)
			rec_data[i] = NULL;
		dec_ref(o);
	}
	return rela;
}

int __inline filter_cmp(void* value, void* target, int opertype,int datatype) {
	switch (opertype)
	{
	case EQUAL:
		switch (datatype)
		{
		case INT:
			return *(int*)value == *(int*)target;
		case REAL:
			return *(float*)value == *(float*)target;
		default:
			return strcmp(value, target) == 0 ? 1 : 0;
		}
	case LESSTHAN:
		switch (datatype)
		{
		case INT:
			return (*(int*)value < *(int*)target) ? 1 : 0;
		case REAL:
			return (*(float*)value < *(float*)target) ? 1 : 0;
		default:
			return strcmp(value, target) < 0 ? 1 : 0;
		}
	case GREATERTHAN:
		switch (datatype)
		{
		case INT:
			return (*(int*)value > *(int*)target) ? 1 : 0;
		case REAL:
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

__inline Relation* rela_filter_col_key(Relation* rela, Pair* schema, void* target,int opertype) {
	struct rehead* col_ = rela_search_col(rela, schema);
	Vector* data_set = &rela->data_set;
	int offset_ = col_->rec_offset;
	int data_type = col_->data_type;
	int len_ = data_set->usedsize_;
	void** rec_data = data_set->vector_;
	for (int i = 0; i < len_; i++) {
		obj* o = rec_data[i];
		if (!o) continue;
		unsigned char* data_ = o->data_;
		if (filter_cmp(data_ + offset_, target, opertype, data_type))
			continue;
		if (o->ref_count == 1)
			rec_data[i] = NULL;
		dec_ref(o);
	}
	return rela;
}
