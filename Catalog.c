#include"Catalog.h"
#include"Mem\MemPool.h"

int db_match_name(DBnode* node, const char* key){
	if (node->name_ == NULL || key == NULL)
		return -1;
	return strcmp(node->name_, key);
}

void* dbnode_search(void* head,const char* name){
	return list_search(head, name, db_match_name);
}

Table * db_get_table(DBnode * db, char * tablename){
	return vector_search(&db->tables, tablename, table_cmp_name);
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

int col_cmp_name(Column * col1, Column * col2){
	return strcmp(col1->column_name, col2->column_name);
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
	case REAL: {
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
	VectorSetCompMethod(&t->cols,col_cmp_name);
	VECTOR_INIT_LEN(&t->cols, V_INIT_LEN);
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

void table_del(Table * table){
	VECTOR_FREE(&table->cols);
	mem_free(table);
}

int table_cmp_name(Table * t1, Table * t2){
	return strcmp(t1->t_info.table_name,t2->t_info.table_name);
}

Column * table_get_col(Table * t, char* colname){
	return vector_search(&t->cols, colname, col_cmp_name);
}

void table_add_col(Table * t, Column * col){
	VECTOR_PUSHBACK(&t->cols, col);
	t->t_info.table_col_count++;
	col->column_rec_offset = t->t_info.table_data_len;
	t->t_info.table_data_len += col->column_data_len;
}





