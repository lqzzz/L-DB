#include"Sqlparse.h"

typedef struct insert{
	Vector insert_rows;
	char* table_name;
	SelectNode* select_node;
}InsertNode;

InsertNode* new_insert_node(const char* tablename) {
	InsertNode* insert = mem_alloc(sizeof(*insert));
	VECTOR_INIT(&insert->insert_rows);
	insert->table_name = tablename;
	return insert;
}
int parse_insert_values(char* errmsg, DBnode* db, Table* table, Vector* collist, Token** curr, QueryNode** pnode) {
	if (MOVE_NEXT_TOKEN_TYPE != LB)
		PARSE_ERROR("ȱ�� �� ");
	NEXT_TOKEN;

	int fill_rec_count = 0;
	int col_count = VectorGetUsedSize(collist);
	int index_ = 0;
	size_t data_len = table->t_info.table_data_len;

	InsertNode* insert = new_insert_node(table->t_info.table_name);

	*pnode = new_insert_query(insert);

	Vector *row_v = &insert->insert_rows;
	char* row_ = mem_calloc(1, data_len);

	for (;;) {
		Column* col_ = VECTOR_GET_VALUE(collist, index_);
		int datatype = col_->column_data_type;
		index_++;
		if (!((TOKEN_TYPE == datatype) || 
			   TOKEN_TYPE == TEXT && datatype == CHAR ))
			PARSE_ERROR("�������Ͳ�ƥ��");

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
				PARSE_ERROR("insert���������ĿС��values�����ָ��ֵ����Ŀ,VALUES �Ӿ���ֵ����Ŀ������ INSERT �����ָ�����е���Ŀƥ��");
			NEXT_TOKEN;
		}else if (TOKEN_TYPE == RB) {
			if (index_ != col_count)
				PARSE_ERROR("insert���������Ŀ����values�����ָ��ֵ����Ŀ,VALUES �Ӿ���ֵ����Ŀ������ INSERT �����ָ�����е���Ŀƥ��");
			fill_rec_count++;
			index_ = 0;
			VECTOR_PUSHBACK(row_v, row_);
			row_ = mem_calloc(1, data_len);

			if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
				if (MOVE_NEXT_TOKEN_TYPE != LB)
					PARSE_ERROR("ȱ�� ( ");
				NEXT_TOKEN;
			}else 
				break;
		}else
			PARSE_ERROR("ȱ�� , �� )");
	}
	return SQL_OK;
ERROR:
	return SQL_ERROR;
}


//int parse_insert_values(char* errmsg,DBnode* db,Table* table,Vector* collist,Token** curr,QueryNode** pnode) {
//	if (MOVE_NEXT_TOKEN_TYPE != LB)
//		PARSE_ERROR("ȱ�� �� ");
//	NEXT_TOKEN;
//
//	int fill_rec_count = 0;
//	int col_count = VectorGetUsedSize(collist);
//	int index_ = 0;
//	size_t data_len = table->t_info.table_data_len;
//
//	*pnode = mem_alloc(sizeof(QueryNode));
//	QueryNode* node = *pnode;
//	node->con_type = INSERT;
//	VECTOR_INIT(&node->insert_rows);
//	PAIR_INIT(&node->left_opand);
//	PairSetFirst(&node->left_opand, table->t_info.table_name);
//
//	Vector *row_v = &node->insert_rows;
//	char* row_ = mem_calloc(1, data_len);
//
//	for (;;) {
//		Column* col_ = VECTOR_GET_VALUE(collist, index_);
//		int datatype = col_->column_data_type;
//		index_++;
//		if (!((TOKEN_TYPE == datatype) || 
//			   TOKEN_TYPE == TEXT && datatype == CHAR ))
//			PARSE_ERROR("�������Ͳ�ƥ��");
//
//		void* item_ = (*curr)->value_;
//		size_t item_size = 0;
//
//		if (TOKEN_TYPE == INT)
//			*(int*)(row_ + col_->column_rec_offset) = item_;
//		else if (TOKEN_TYPE == FLOAT)
//			memcpy(row_ + col_->column_rec_offset, &item_, sizeof(float));
//		else
//			memcpy(row_ + col_->column_rec_offset, item_, strlen(item_));
//
//		if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
//			if (index_ == col_count)
//				PARSE_ERROR("insert���������ĿС��values�����ָ��ֵ����Ŀ,VALUES �Ӿ���ֵ����Ŀ������ INSERT �����ָ�����е���Ŀƥ��");
//			NEXT_TOKEN;
//		}else if (TOKEN_TYPE == RB) {
//			if (index_ != col_count)
//				PARSE_ERROR("insert���������Ŀ����values�����ָ��ֵ����Ŀ,VALUES �Ӿ���ֵ����Ŀ������ INSERT �����ָ�����е���Ŀƥ��");
//			fill_rec_count++;
//			index_ = 0;
//			VECTOR_PUSHBACK(row_v, row_);
//			row_ = mem_calloc(1, data_len);
//
//			if (MOVE_NEXT_TOKEN_TYPE == COMMA) {
//				if (MOVE_NEXT_TOKEN_TYPE != LB)
//					PARSE_ERROR("ȱ�� ( ");
//				NEXT_TOKEN;
//			}else 
//				break;
//		}else
//			PARSE_ERROR("ȱ�� , �� )");
//	}
//	return SQL_OK;
//ERROR:
//	return SQL_ERROR;
//}

int parse_insert(char* errmsg,DBnode* dbnode, Token** curr,QueryNode** pnode) {
	Table* table_;
	int flag = 0;
	if ((table_ = db_get_table(dbnode, (*curr)->value_)) == NULL) 
		PARSE_ERROR("��Ч����");
	flag = MOVE_NEXT_TOKEN_TYPE;

	Vector v_col;
	switch (flag){
	case LB:
		VECTOR_INIT(&v_col);
		if (MOVE_NEXT_TOKEN_TYPE != ID) 
			PARSE_ERROR("ȱ��ID");

		Column *col_;
		for (;;) {
			if ((col_ = table_get_col(table_, (*curr)->value_)) == NULL) 
				PARSE_ERROR("�޶�Ӧ����");
			VECTOR_PUSHBACK(&v_col, col_);

			switch (MOVE_NEXT_TOKEN_TYPE) {
			case COMMA:
				if (MOVE_NEXT_TOKEN_TYPE != ID) 
					PARSE_ERROR("ȱ��ID");
				break;
			case RB:
				;
				int res = parse_insert_values(errmsg, dbnode, table_, 
					&v_col, curr, pnode);
				vector_del_nofree(&v_col);
				return res;
			default: 
				PARSE_ERROR("�޶�Ӧ����");
			}
		}
	case VALUES:
		return parse_insert_values(errmsg, dbnode, table_, &table_->cols, curr, pnode);
	default:
		PARSE_ERROR("ȱ��ID ���� 'values' ");
	}
ERROR:
	if (flag == RB)
		vector_del_nofree(&v_col);
	return SQL_ERROR;
}
