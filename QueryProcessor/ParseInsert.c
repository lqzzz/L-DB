#include"Sqlparse.h"

typedef struct insert{
	DBitems *set_cols, *insert_items, table_item;
	char* insert_row;
	Vector insert_rows;
	//char* table_name;
	SelectNode* select_node;
}InsertNode;

InsertNode* new_insert_node(void) {
	InsertNode* insert = mem_alloc(sizeof(InsertNode));
	VECTOR_INIT(&insert->insert_rows);
	return insert;
}

int parse_insert_values(char* errmsg, DBnode* db,InsertNode* insertnode, Token** curr) {
	size_t set_cols_len, insert_items_len, data_len;
	char* row_;
	if (MOVE_NEXT_TOKEN_TYPE != LB)
		PARSE_ERROR("缺少 （ ");
	NEXT_TOKEN;

	if (get_item_list(errmsg, &insertnode->insert_items, db, curr, INSERT_ITEM) == SQL_ERROR)
		goto ERROR;

	set_cols_len = list_len(&insertnode->set_cols->head);
	insert_items_len = list_len(&insertnode->insert_items->head);

	if (set_cols_len < insert_items_len)
		PARSE_ERROR("insert语句中列数目小于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");
	if (set_cols_len > insert_items_len)
		PARSE_ERROR("insert语句中列数目大于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");

	data_len = insertnode->table_item.table_->t_info.table_data_len;
	row_ = insertnode->insert_row = mem_calloc(1, data_len);

	DBitems* dbitem;
	LIST_FOREACH(dbitem, insertnode->set_cols,
		Column* col = dbitem->col_;
		enum TokenType datatype = col->column_data_type;
		size_t offset = col->column_rec_offset;
		char* item_ = insertnode->insert_items->base_item;

		if (!((TOKEN_TYPE == datatype) || 
			   TOKEN_TYPE == TEXT && datatype == CHAR ))
			PARSE_ERROR("数据类型不匹配");
		if (TOKEN_TYPE == INT)
			*(int*)(row_ + offset) = item_;
		else if (TOKEN_TYPE == FLOAT)
			memcpy(row_ + offset, &item_, sizeof(float));
		else
			memcpy(row_ + offset, item_, strlen(item_));
		);

	return SQL_OK;
ERROR:
	return SQL_ERROR;
}


int parse_insert(char* errmsg,DBnode* db, Token** curr,QueryNode** pnode) {
	*pnode = new_insert_query();
	Table* table_;
	if ((table_ = db_get_table(db, (*curr)->value_)) == NULL) 
		PARSE_ERROR("无效表名");
	(*pnode)->insert_node->table_item.table_ = table_;

	DBitems* set_cols = (*pnode)->insert_node->set_cols;
	DBitems* table_item = &(*pnode)->insert_node->table_item;

	switch (MOVE_NEXT_TOKEN_TYPE) {
	case LB:
		NEXT_TOKEN;
		if (get_item_list(errmsg, &(*pnode)->insert_node->set_cols, db, curr, 0) == SQL_ERROR)
			goto ERROR;
		if (check_item_list(errmsg, db, set_cols, table_item) == SQL_ERROR)
			goto ERROR;
		if (TOKEN_TYPE != RB)
			PARSE_ERROR("缺少 ) ");
		if (parse_insert_values(errmsg, db, (*pnode)->insert_node, curr) == SQL_ERROR)
			goto ERROR;
		break;
	case VALUES:
		if (parse_insert_values(errmsg, db, (*pnode)->insert_node, curr) == SQL_ERROR)
			goto ERROR;
		break;
	default:
		PARSE_ERROR("缺少ID 或者 'values' ");
	}

	return SQL_OK;
ERROR:
	return SQL_ERROR;
}
	//int fill_rec_count = 0;
	//int col_count = VectorGetUsedSize(collist);
	//int index_ = 0;
	//size_t data_len = table->t_info.table_data_len;
	//char* row_ = mem_calloc(1, data_len);

	//Vector *row_v = &(*pnode)->insert_rows;


	//


	//for (;;) {
	//	Column* col_ = VECTOR_GET_VALUE(collist, index_);
	//	int datatype = col_->column_data_type;
	//	index_++;

	//	if (!((TOKEN_TYPE == datatype) || 
	//		   TOKEN_TYPE == TEXT && datatype == CHAR ))
	//		PARSE_ERROR("数据类型不匹配");

	//	void* item_ = (*curr)->value_;
	//	size_t item_size = 0;

	//	if (TOKEN_TYPE == INT)
	//		*(int*)(row_ + col_->column_rec_offset) = item_;
	//	else if (TOKEN_TYPE == FLOAT)
	//		memcpy(row_ + col_->column_rec_offset, &item_, sizeof(float));
	//	else
	//		memcpy(row_ + col_->column_rec_offset, item_, strlen(item_));

	//	if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
	//		if (index_ == col_count)
	//			PARSE_ERROR("insert语句中列数目小于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");
	//		NEXT_TOKEN;
	//	}else if (TOKEN_TYPE == RB) {
	//		if (index_ != col_count)
	//			PARSE_ERROR("insert语句中列数目大于values语句中指定值的数目,VALUES 子句中值的数目必须与 INSERT 语句中指定的列的数目匹配");
	//		fill_rec_count++;
	//		index_ = 0;
	//		VECTOR_PUSHBACK(row_v, row_);
	//		row_ = mem_calloc(1, data_len);

	//		if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
	//			if (MOVE_NEXT_TOKEN_TYPE != LB)
	//				PARSE_ERROR("缺少 ( ");
	//			NEXT_TOKEN;
	//		}else 
	//			break;
	//	}else
	//		PARSE_ERROR("缺少 , 或 )");
	//}
