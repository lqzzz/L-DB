#include <stdio.h>

#include "Scanner.h"
#include "Relation.h"
#include "../BaseStruct/Listhead.h"
#include "../Catalog.h"
#include "../BaseStruct/Vector.h"
#include "../Mem/MemPool.h"
#include "../BaseStruct/Pair.h"
#include "../StorageEngine/BufferManager.h"
#include"Sqlparse.h"`
#define SQL_ERROR -1
#define SQL_OK 1
#define PARSE_ERROR(buf) do{\
sprintf(errmsg, " %d 行, %d 列 错误：%s\n", (*curr)->l_num, (*curr)->c_num, buf); goto ERROR;\
}while(0)

static int parse_create(char* errmsg,DBnode* dbnode, Token** curr);
static Table* parse_create_table(char* errmsg,DBnode *dbnode,Token** token);
static int parse_create_column(char* errmsg,Table* t,Token** token);
static int parse_datatype(char* errmsg,int datatype, Token**);

static int parse_insert(char* errmsg, DBnode*, Token**,QueryNode**);
static int parse_insert_values(char* errmsg, DBnode*, Table*, Vector*, Token**,QueryNode**);

int parse_insert_values(char* errmsg,DBnode* db,Table* table,Vector* collist,Token** curr,QueryNode** pnode) {
	if (MOVE_NEXT_TOKEN_TYPE != LB)
		PARSE_ERROR("缺少 （ ");
	NEXT_TOKEN;

	int fill_rec_count = 0;
	int col_count = VectorGetUsedSize(collist);
	int index_ = 0;
	size_t data_len = table->t_info.table_data_len;

	*pnode = mem_alloc(sizeof(QueryNode));
	QueryNode* node = *pnode;
	node->con_type = INSERT;
	VECTOR_INIT(&node->inset_rows);
	PAIR_INIT(&node->left_opand);
	PairSetFirst(&node->left_opand, table->t_info.table_name);

	Vector *row_v = &node->inset_rows;
	char* row_ = mem_calloc(1, data_len);

	for (;;) {
		Column* col_ = VECTOR_GET_VALUE(collist, index_);
		int datatype = col_->column_data_type;
		index_++;
		if (!((TOKEN_TYPE == datatype) || 
			   TOKEN_TYPE == TEXT && datatype == CHAR ))
			PARSE_ERROR("数据类型不匹配");

		void* item_ = (*curr)->value_;
		size_t item_size = 0;

		if (TOKEN_TYPE == INT)
			*(int*)(row_ + col_->column_rec_offset) = item_;
		else if (TOKEN_TYPE == FLOAT)
			memcpy(row_ + col_->column_rec_offset, &item_, sizeof(float));
		else
			memcpy(row_ + col_->column_rec_offset, item_, strlen(item_));

		if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
			if (index_ == col_count)
				PARSE_ERROR("insert语句中列数目小于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");
			NEXT_TOKEN;
		}else if (TOKEN_TYPE == RB) {
			if (index_ != col_count)
				PARSE_ERROR("insert语句中列数目大于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");
			fill_rec_count++;
			index_ = 0;
			VECTOR_PUSHBACK(row_v, row_);
			row_ = mem_calloc(1, data_len);

			if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
				if (MOVE_NEXT_TOKEN_TYPE != LB)
					PARSE_ERROR("缺少 ( ");
				NEXT_TOKEN;
			}else 
				break;
		}else
			PARSE_ERROR("缺少 , 或 )");
	}
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}

int parse_insert(char* errmsg,DBnode* dbnode, Token** curr,QueryNode** pnode) {
	Table* table_;
	int flag = 0;
	if ((table_ = db_get_table(dbnode, (*curr)->value_)) == NULL) 
		PARSE_ERROR("无效表名");
	flag = MOVE_NEXT_TOKEN_TYPE;

	Vector v_col;
	switch (flag){
	case LB:
		VECTOR_INIT(&v_col);
		if (MOVE_NEXT_TOKEN_TYPE != ID) 
			PARSE_ERROR("缺少ID");

		Column *col_;
		for (;;) {
			if ((col_ = table_get_col(table_, (*curr)->value_)) == NULL) 
				PARSE_ERROR("无对应列名");
			VECTOR_PUSHBACK(&v_col, col_);

			switch (MOVE_NEXT_TOKEN_TYPE) {
			case COMMA:
				if (MOVE_NEXT_TOKEN_TYPE != ID) 
					PARSE_ERROR("缺少ID");
				break;
			case RB:
				;
				int res = parse_insert_values(errmsg, dbnode, table_, 
					&v_col, curr, pnode);
				vector_del_nofree(&v_col);
				return res;
			default: 
				PARSE_ERROR("无对应列名");
			}
		}
	case VALUES:
		return parse_insert_values(errmsg, dbnode, table_, &table_->cols, curr, pnode);
	default:
		PARSE_ERROR("缺少ID 或者 'values' ");
	}
ERROR:
	if (flag == RB)
		vector_del_nofree(&v_col);
	return SQL_ERROR;
}

Table* parse_create_table(char* errmsg,DBnode *dbnode, Token** curr) {
	if (TOKEN_TYPE != ID) 
		PARSE_ERROR("缺少表名");

	char* tablename = (*curr)->value_;
	if(db_get_table(dbnode, tablename))
		PARSE_ERROR("该表已存在");
	Table* table = new_table(tablename, dbnode->name_, dbnode->table_count);
	TableInfo* info = &table->t_info;

	if (MOVE_NEXT_TOKEN_TYPE != LB) 
		PARSE_ERROR("缺少左括号");
	
	if (parse_create_column(errmsg,table, curr) == SQL_ERROR) 
		goto ERROR;

	return table;
ERROR:
	return SQL_ERROR;
}

int parse_create(char* errmsg,DBnode* dbnode, Token** curr) {
	Table* table_ = NULL;
	switch (TOKEN_TYPE)
	{
	case TABLE:
		NEXT_TOKEN;
		if ((table_ = parse_create_table(errmsg,dbnode, curr)) == SQL_ERROR)
			goto ERROR;
		db_add_table(dbnode, table_);

		FileHeadData* fhd = new_file_head_data(PageCount, table_->t_info.table_page_solt_count, 
			table_->t_info.table_data_len);
		FHead* fh = new_file_head(table_->t_info.table_name, fhd);
		init_file(fh);
		bm_add_file_head(dbnode->id_, fh);

		return SQL_OK;
	case DATABASE:
		NEXT_TOKEN;
		if (TOKEN_TYPE != ID) 
			PARSE_ERROR("缺少数据库名");
		char* db_name = (*curr)->value_;
		if (DBNODE_SEARCH(dbnode, db_name))
			PARSE_ERROR("该数据库已存在");
		NEXT_TOKEN;
		DBnode* db_ = database_create(db_name, ((DBnode*)dbnode->list_head.prve_)->id_ + 1, 0);
		new_bufferManager(db_);
		if(dbnode)
			LIST_ADD_TAIL(&dbnode->list_head, &db_->list_head);
		return SQL_OK;
	default:
		PARSE_ERROR("创建节点无效");
	}
ERROR:
	return SQL_ERROR;
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
			//if (parse_select(dbnode, curr) == -1)
			//	goto ERROR;
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

int parse_datatype(char* errmsg,int datatype, Token** curr) {
	switch (datatype) {
	case INT:
		return 4;
	case CHAR:
		if (NEXT_TOKEN_TYPE != LB) {
			return 4;
		}
		NEXT_TOKEN;
		if(MOVE_NEXT_TOKEN_TYPE != INT)
			PARSE_ERROR("缺少数据长度");	

		int len = (*curr)->value_;

		(*curr)->value_ = NULL;

		if (MOVE_NEXT_TOKEN_TYPE != RB)
			PARSE_ERROR("缺少 ) ");

		return len;
	case FLOAT:
		return 4;
	default:
		goto ERROR;
		break;
	}
ERROR:
	return SQL_ERROR;
}

int parse_create_column(char* errmsg,Table* t, Token** curr) {
	char* table_name = t->t_info.table_name;
	char* db_name = t->t_info.table_db_name;
	for (;;) {
		if (MOVE_NEXT_TOKEN_TYPE != ID)
			PARSE_ERROR("缺少列名");

		char* col_name = (*curr)->value_;
		int col_id = t->t_info.table_col_count;
		int data_type = MOVE_NEXT_TOKEN_TYPE;
		int data_len = parse_datatype(errmsg,data_type, curr);
		if (data_len == SQL_ERROR)
			PARSE_ERROR("数据类型错误");

		table_add_col(t,new_column(col_name, table_name, db_name, col_id, data_type, data_len));

		switch (MOVE_NEXT_TOKEN_TYPE)
		{
		case COMMA:
			//NEXT_TOKEN;
			continue;
		case RB:
			NEXT_TOKEN;
			return col_id;
		default:
			PARSE_ERROR("缺少 ',' 或 'NOT' 或 ')'");
		}
	}

ERROR:
	return -1;
}

