#include"Catalog.h"
#include"Mem\MemPool.h"
#include<stdio.h>
int db_match_name(DBnode* node, const char* key){
	if (node->name_ == NULL || key == NULL)
		return -1;
	return strcmp(node->name_, key);
}

void* dbnode_search(void* head,const char* name){
	return list_search(head, name, db_match_name);
}

Table* db_get_table(DBnode * db, char * tablename){
	return vector_search(&db->tables, tablename, table_match_name);
}

void db_add_table(DBnode * db, Table * t){
	VECTOR_PUSHBACK(&db->tables, t);
	db->table_count++;
}

DBnode* database_create(char* name,size_t id,size_t tablecount){
	DBnode* dbnode = mem_calloc(1,sizeof(DBnode));
	strcpy(dbnode->name_, name);
	dbnode->id_ = id;
	dbnode->table_count = tablecount;
	VECTOR_INIT(&dbnode->tables);
	LIST_INIT(dbnode);
	return dbnode;
}

Column* new_column(char* colname, char* tablename, char*dbname, size_t id,enum TokenType DT,size_t datalen){
	Column* col = mem_calloc(1,sizeof(Column));
	strcpy(col->column_name, colname);
	strcpy(col->column_table_name, tablename);
	strcpy(col->column_db_name, dbname);
	col->column_data_type = DT;
	col->column_num = id;
	col->column_data_len = datalen;
	return col;
}

void col_set_info(Column * col, size_t column_num, enum TokenType column_data_type, size_t column_not_null, size_t column_unique, size_t column_rec_offset,size_t datalen, char * column_name, char * column_table_name, char * column_db_name){
	if(column_name)
		strcpy(col->column_name, column_name);
	if(column_table_name)
		strcpy(col->column_table_name, column_table_name);
	if(column_db_name)
		strcpy(col->column_db_name, column_db_name);
	col->column_data_type= column_data_type;
	col->column_num = column_num;
	col->column_rec_offset = column_rec_offset;
	col->column_not_null = column_not_null;
	col->column_unique = column_unique;
	col->column_data_len = datalen;
}

int col_match_name(Column * col1, const char* name){
	return strcmp(col1->column_name, name);
}

void col_del(Column* col){
	mem_free(col);
}

size_t get_max_data_len(void * item, enum Tokentype datatype,size_t currmaxlen) {
	size_t max_len = currmaxlen;
	size_t item_len = 0;
	switch (datatype) {
	case INT: {
		int num_ = *(int*)(item);
		while (num_ != 0) {
			num_ = num_ / 10;
			item_len++;
		}
		if (item_len > max_len)
			max_len = item_len;
	}
	break;
	case FLOAT: {
		int num_ = *(float*)(item);
		while (num_ != 0) {
			num_ = num_ / 10;
			item_len++;
		}
		item_len += 2;
		if (item_len > max_len)
			max_len = item_len;
	}
		break;
	default:
		item_len = strlen(item);
		if (item_len > max_len)
			max_len = item_len;
		break;
	}
	return max_len;
}


void database_del(DBnode * db){
	
}

void rec_del(Record * rec){
	mem_free(rec->time_stamp);
	mem_free(rec->data_);
	mem_free(rec);
}


Record* rec_create(char* schemaname,size_t len){
	Record* rec_ = mem_alloc(sizeof(Record));
	rec_->time_stamp = NULL;
	rec_->schema_name = schemaname;
	rec_->data_ = mem_alloc(len);
	return rec_;
}

void* rec_dup_data(Record * rec,size_t datalen){
	void* data_ = mem_alloc(datalen);
	memcpy(data_, rec->data_, datalen);
	return data_;
}

Table* new_table(char* tablename,char* dbname,size_t id){
	Table* t = mem_calloc(1, sizeof(Table));
	strcpy(t->t_info.table_name, tablename);
	strcpy(t->t_info.table_db_name, dbname);
	t->t_info.table_num = id;
	VectorSetFreeMethod(&t->cols,col_del);
	//VectorSetCompMethod(&t->cols,col_cmp_name);
	VECTOR_INIT_LEN(&t->cols, V_INIT_LEN);
	return t;
}

Table* new_join_table(Table* l, Table* r){
	Table* t = new_table("", l->t_info.table_db_name, 0);
	TableInfo* il = &l->t_info;
	TableInfo* ir = &r->t_info;
	TableInfo* jr = &t->t_info;
	jr->table_auto_increment =
		il->table_auto_increment + ir->table_auto_increment;
	jr->table_col_count = il->table_col_count + ir->table_col_count;
	jr->table_data_len = il->table_data_len + ir->table_data_len;
	jr->table_rec_size = 0;
	jr->table_page_solt_count = 0;
	Column* col;
	Column* new_col;
	VECTOR_FOREACH(col, &l->cols,
		new_col = mem_alloc(sizeof(Column));
		*new_col = *col;
		table_add_col(t, new_col);
		);

	VECTOR_FOREACH(col, &r->cols,
		new_col = mem_alloc(sizeof(Column));
		*new_col = *col;
		new_col->column_rec_offset += il->table_data_len;
		new_col->column_num += il->table_col_count;
		table_add_col(t, new_col);
		);
	return t;
}


void table_init(Table* table, char * name, size_t id, size_t datalen, size_t columncount, size_t recsize, size_t pageslotcount){
	TableInfo* info = &table->t_info;
	strcpy(info->table_name, name);
	info->table_num = id;
	info->table_data_len= datalen;
	info->table_col_count= columncount;
	info->table_rec_size= recsize;
	info->table_page_solt_count = pageslotcount;
	VECTOR_INIT_LEN(&table->cols, columncount);
}

void db_head_print(void) {
	for (size_t i = 0; i < 3; i++){
		printf("+");
		int len = 0;
		switch (i)
		{
		case 0:
			len = 2;
			break;
		case 1:
			len = 6;
			break;
		default:
			len = 8;
			break;
		}
		for (size_t i = 0; i < len; i++)
			printf("-");
	}
	printf("+\n");

	for (size_t i = 0; i < 3; i++){
		int len = 0;
		char* str = NULL;
		switch (i)
		{
		case 0:
			len = 2;
			str = "ID";
			break;
		case 1:
			len = 6;
			str = "表数量";
			break;
		default:
			len = 8;
			str = "数据库名";
			break;
		}
		printf("|%-*s", len, str);
	}
	printf("|\n");
}

void db_print(DBnode* dbh) {
	DBnode* db = NULL;
	db_head_print();
	LIST_FOREACH(db, dbh,
	for (size_t i = 0; i < 3; i++){
		int len = 0;
		char* str = NULL;
		switch (i)
		{
		case 0:
			len = 2;
			str = db->id_;
			printf("|%-*d", len, str);
			break;
		case 1:
			len = 6;
			str = db->table_count;
			printf("|%-*d", len, str);
			break;
		default:
			len = 8;
			str = db->name_;
			printf("|%-*s", len, str);
			break;
		}
	}
	printf("|\n");
	);
	printf("\n");
}

void print_head_format() {

}

void table_print(DBnode* db){
	for (size_t i = 0; i < 8; i++){
		printf("+");
		int len = 0;
		switch (i)
		{
		case 0:

			break;
		case 1:

			break;
		case 2:

			break;
		case 3:

			break;
		case 4:

			break;
		case 5:

			break;
		case 6:

			break;
		case 7:

			break;
		default:

			break;
		}

		for (int i = 0; i < len; i++)
			printf("-");
		
	}
	printf("+\n");

	for (size_t i = 0; i < 8; i++){
		printf("+");
		int maxlen;
		char* itemname;
		switch (i)
		{
		case 0:
			itemname = "ID";
			maxlen = 2;
			break;
		case 1:
			itemname = "表名";
			//maxlen = 4;
			break;
		case 2:
			itemname = "列数目";
			maxlen = 3;
			break;
		case 3:
			itemname = "每行长度";
			maxlen = 4;
			break;
		case 4:
			itemname = "行数据量";
			maxlen = 4;
			break;
		case 5:
			itemname = "每页行数";
			maxlen = 4;
			break;
		case 6:
			itemname = "所属数据库";
			//maxlen = 
			break;
		default:

			break;
		}

		//for (int i = 0; i < len; i++)
		//	printf("-");

	}
	printf("+\n");
}

void table_del(Table * table){
	VECTOR_FREE(&table->cols);
	mem_free(table);
}

int table_match_name(Table * t1, const char* name){
	return strcmp(t1->t_info.table_name,name);
}

Column* table_get_col(Table* t, const char* colname){
	return vector_search(&t->cols, colname, col_match_name);
}

void table_add_col(Table * t, Column * col){
	VECTOR_PUSHBACK(&t->cols, col);
	t->t_info.table_col_count++;
	col->column_rec_offset = t->t_info.table_data_len;
	t->t_info.table_data_len += col->column_data_len;
}





