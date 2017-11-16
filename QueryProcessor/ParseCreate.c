#include"Sqlparse.h"
#include"../StorageEngine/BufferManager.h"
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
		bm_add_raw_file_head(dbnode->id_, fh);

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

		int len = (int)(*curr)->value_;

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
