#include <stdio.h>

#include "Scanner.h"
#include "Relation.h"
#include "../BaseStruct/Listhead.h"
#include "../Catalog.h"
#include "../BaseStruct/Vector.h"
#include "../Mem/MemPool.h"
#include "../BaseStruct/Pair.h"
#include "../StorageEngine/BufferManager.h"

extern int parse_create(DBnode* dbnode, Token** curr);
extern Table* parse_create_table(DBnode *dbnode,Token** token);
extern Column* parse_create_column(DBnode *dbnode,Token** token);
extern int parse_datatype(Column*, int datatype, Token**);
extern int parse_insert(DBnode*, Token**);
extern int parse_insert_values(DBnode*,Table*, Vector*, Token**);

int parse_insert_values(DBnode* db,Table* table,Vector* collist,Token** curr) {
	if (TOKEN_TYPE != LB) PARSE_ERROR("缺少 （ ");
	NEXT_TOKEN;

	int fill_rec_count = 0;
	int col_count = VectorGetUsedSize(collist);
	int index_ = 0;
	size_t data_len = table->data_len;
	Vector row_v;
	VECTOR_INIT(&row_v, INIT_LEN);
	char* row_ = mem_calloc(1, table->data_len);

	for (;;) {
		Column* col_ = VECTOR_GET_VALUE(collist, index_);
		index_++;
		if (!((TOKEN_TYPE == col_->data_type) ||
			TOKEN_TYPE == TEXT &&
			(col_->data_type == CHAR ||
				col_->data_type == VARCHAR)))
			PARSE_ERROR("数据类型不匹配");

		void* item_ = NULL;
		move_value(&(*curr)->value_, &item_);
		size_t item_size = sizeof(size_t);
		if (TOKEN_TYPE == INT || TOKEN_TYPE == REAL)
			item_size = sizeof(size_t);
		else item_size = strlen(item_);

		col_->max_data_len = get_max_data_len(item_,
			TOKEN_TYPE, col_->max_data_len);

		memcpy(row_ + col_->rec_offset, item_, item_size);

		if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
			if (index_ == col_count)
				PARSE_ERROR("insert语句中列数目小于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");
			NEXT_TOKEN;
		}else if (TOKEN_TYPE == RB) {
			if (index_ != col_count)
				PARSE_ERROR("insert语句中列数目大于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");
			fill_rec_count++;
			index_ = 0;
			VECTOR_PUSHBACK(&row_v, row_);
			row_ = mem_calloc(1, table->data_len);

			if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
				if (MOVE_NEXT_TOKEN_TYPE != LB)
					PARSE_ERROR("缺少 ( ");
				NEXT_TOKEN;
			}else 
				break;
		}else
			PARSE_ERROR("缺少 , 或 )");
	}
	VectorIter iter_;
	vector_init_iter(&row_v, &iter_);
	page_fill(get_buffman(db->id_), table->name_, &iter_);
	VECTOR_CLEAR_VAL(&row_v);
	printf("%d行受影响\n", fill_rec_count);
	return 0;
ERROR:
	return -1;
}

static int parse_insert(DBnode* dbnode, Token** curr) {
	Table* table_;
	if ((table_ = DBNODE_SEARCH(dbnode->table_head, (*curr)->value_)) == NULL) 
		PARSE_ERROR("无效表名");
	Vector v_col;
	VECTOR_INIT(&v_col, INIT_LEN);
	switch (MOVE_NEXT_TOKEN_TYPE){
	case LB:
		if (MOVE_NEXT_TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
		Column *col_;
		for (;;) {
			if ((col_ = DBNODE_SEARCH(table_->col_head, (*curr)->value_)) == NULL) 
				PARSE_ERROR("无对应列名");
			VECTOR_PUSHBACK(&v_col, col_);

			switch (MOVE_NEXT_TOKEN_TYPE) {
			case COMMA:
				if (MOVE_NEXT_TOKEN_TYPE != ID) 
					PARSE_ERROR("缺少ID");
				break;
			case RB:
				if (MOVE_NEXT_TOKEN_TYPE != VALUES) 
					PARSE_ERROR("缺少values");
				NEXT_TOKEN;
				if ((parse_insert_values(dbnode,table_, &v_col, curr) == -1))
					goto ERROR;
				return 1;
			default: PARSE_ERROR("无对应列名");
			}
		}
	case VALUES:
		//TODO
		//if ((parse_insert_values(dbnode, table_, curr, fill_field_count)) == -1) goto ERROR;
		return 0;
	default:
		PARSE_ERROR("缺少ID");
	}
ERROR:
	return -1;
}

Table* parse_create_table(DBnode *dbnode, Token** curr) {
	Table* table_ = NULL;
	if (TOKEN_TYPE != ID) PARSE_ERROR("缺少表名");
	if (dbnode->table_head != NULL) {
		if (DBNODE_SEARCH(dbnode->table_head, (*curr)->value_) != NULL)
			PARSE_ERROR("该表已存在");
	}
	size_t db_name_len = strlen(dbnode->name_) + 1;
	char* db_name = mem_alloc(db_name_len);
	strcpy(db_name, dbnode->name_);
	table_ = table_create();
	char* table_name = NULL;
	move_value(&(*curr)->value_, &table_name);
	if (MOVE_NEXT_TOKEN_TYPE != LB) PARSE_ERROR("缺少左括号");
	if (MOVE_NEXT_TOKEN_TYPE != ID) PARSE_ERROR("缺少列名");
	if ((table_->col_head = parse_create_column(dbnode, curr)) == NULL) goto ERROR;
	table_->column_count = ((Column*)(LIST_GET_PRVE(table_->col_head)))->num_ + 1;
	Column* col_ = NULL;
	int offset_ = 0;
	LIST_FOREACH(col_,table_->col_head,
	col_->rec_offset = offset_;
	offset_ += col_->data_len;
	col_->table_name = table_name;
	col_->db_name = db_name;
	);
	SIZE_ALINE(offset_);
	table_->data_len = offset_;
	table_->db_name = db_name;
	table_->name_ = table_name;
	if (dbnode->table_head == NULL)
		table_->num_ = 0;
	else 
		table_->num_ = ((Table*)(LIST_GET_PRVE(dbnode->table_head)))->num_ + 1;


	return table_;
ERROR:
	if (table_ != NULL)
		table_del(table_);
	return NULL;
}

static int parse_create(DBnode* dbnode, Token** curr) {
	Table* table_ = NULL;
	switch (TOKEN_TYPE)
	{
	case TABLE:
		NEXT_TOKEN;
		if ((table_ = parse_create_table(dbnode, curr)) == NULL)
			goto ERROR;
		if (dbnode->table_head == NULL)
			dbnode->table_head = table_;
		else
			DB_ADD_TABLE(dbnode, table_);

		FileHeadData* fhd = new_file_head_data(PageCount, table_->page_slot_count, table_->data_len);
		FHead* fh = new_file_head(table_->name_, fhd);
		init_file(fh);
		bm_add_file_head(dbnode->id_, fh);

		table_info_update(table_);

		return 1;
	case DATABASE:
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID) PARSE_ERROR("缺少数据库名");

		char* db_name = NULL;
		move_value(&(*curr)->value_, &db_name);
		if (DBNODE_SEARCH(dbnode, db_name))
			PARSE_ERROR("该数据库已存在");
		NEXT_TOKEN;
		DBnode* db_ = database_create(db_name,((DBnode*)dbnode->list_head.prve_)->id_ + 1);
		new_bufferManager(db_);
		LIST_ADD_TAIL(&dbnode->list_head, &db_->list_head);
		database_info_update(db_);
		return 1;
	default:
		PARSE_ERROR("创建节点无效");
		break;
	}
ERROR:
	return NULL;
}

int sql_parse(DBnode *db, Srcstream* stream) {
	Token* token_head;
	if ((token_head = get_next_token(stream)) == NULL) 
		return NULL;
	for (Token *token_next = get_next_token(stream), *head = &token_head->list_head;
		token_next; token_next = get_next_token(stream)) {
		LIST_ADD_TAIL(&head->list_head, &token_next->list_head);
	}
	Token *t_curr = token_head;
	Token **curr = &t_curr;
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
			if (parse_create(dbnode, curr) == NULL)
				goto ERROR;
			break;
		case SELECT:
			NEXT_TOKEN;
			if (parse_select(dbnode, curr) == -1)
				goto ERROR;
			break;
		case INSERT:
			if (MOVE_NEXT_TOKEN_TYPE != INTO) PARSE_ERROR("缺少INTO");
			if (MOVE_NEXT_TOKEN_TYPE != ID) PARSE_ERROR("缺少ID");
			if (parse_insert(dbnode, curr) == -1) goto ERROR;
			break;
		default:
			break;
		}
	} while (t_curr != token_head);
	Token* token_ = token_head;
	int i = 0;
	LIST_DEL_ALL(token_, &token_head->list_head, token_del(token_););
	return 0;
ERROR: {
	int i = 1;
	Token* token_ = token_head;
	LIST_DEL_ALL(token_, &token_head->list_head,token_del(token_););
	}
	return -1;
}

int parse_datatype(Column *col, int datatype, Token** curr) {
	col->data_type = datatype;
	switch (datatype) {
	case INT:
		col->data_len = sizeof(int);
		break;
	case CHAR:
		if (MOVE_NEXT_TOKEN_TYPE != LB) {
			col->data_len = 1;
			break;
		}else if(MOVE_NEXT_TOKEN_TYPE != INT) {
			PARSE_ERROR("缺少数据长度");
		}else 
			col->data_len = *(int*)(*curr)->value_;
		if (MOVE_NEXT_TOKEN_TYPE != RB)
			PARSE_ERROR("缺少 ) ");
		break;
	case REAL:
		col->data_len = sizeof(float);
		break;
	case TEXT:
		break;
	case TIME:
		break;
	case DATE:
		break;
	case DATETIME:
		break;
	default:
		return 0;
		break;
	}
	switch (MOVE_NEXT_TOKEN_TYPE) {
	case COMMA:
		NEXT_TOKEN;
		break;
	case RB:
		NEXT_TOKEN;
		return 1;
	case NOT:
		break;
	case PRIMARY:
		break;
	case FOREIGN:
		break;
	case DEFAULT:
		break;
	default:
		goto ERROR;
		break;
	}
	return 2;
ERROR:
	return NULL;
}

int parse_not(Column *col, Token** curr) {
	if (TOKEN_TYPE != NULL_) PARSE_ERROR("缺少NULL");
	col->is_null_able = 1;
	switch (MOVE_NEXT_TOKEN_TYPE) {
	case COMMA:
		NEXT_TOKEN;
		break;
	case RB:
		NEXT_TOKEN;
		return 1;
	case NOT:
		break;
	case PRIMARY:
		break;
	case FOREIGN:
		break;
	case DEFAULT:
		break;
	default:
		return 0;
		break;
	}
	return 2;
ERROR:
	return 0;
}

Column* parse_create_column(DBnode *dbnode, Token** curr) {
	int num_ = 0;
	Column *col_head = col_create(num_++);
	Column *col_curr = NULL;
	for(;;)
		switch (TOKEN_TYPE){
		case ID:
			if (col_curr == NULL) col_curr = col_head;
			else {
				col_curr = col_create(num_++);
				LIST_ADD_TAIL(&col_head->list_head, &col_curr->list_head);
				if (DBNODE_SEARCH(col_head, (*curr)->value_) != 0)
					PARSE_ERROR("该列已存在");
			}
			col_curr->max_data_len = get_max_data_len((*curr)->value_, CHAR, 0);
			move_value(&(*curr)->value_, &col_curr->name_);
			
			switch (parse_datatype(col_curr, MOVE_NEXT_TOKEN_TYPE, curr)) {
			case 1:
				return col_head;
			case 0:
				goto ERROR;
			default:
				continue;
			}
			PARSE_ERROR("缺少数据类型");
			goto ERROR;
			break;
		case NOT:
			NEXT_TOKEN;
			switch (parse_not(col_curr, curr)) {
			case 1:
				return col_head;
			case 0:
				goto ERROR;
			default:
				break;
			}
			break;
		default:
			goto ERROR;
			break;
		}
ERROR:
	list_del_all(col_head, col_del);
	return NULL;
}

