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

Table* table_create(){
	Table* t = mem_calloc(1,sizeof(Table));
	LIST_INIT(t);
	return t;
}

DBnode* database_create(char* name,int id){
	DBnode* dbnode = mem_calloc(1,sizeof(DBnode));
	dbnode->name_ = name;
	dbnode->id_ = id;
	LIST_INIT(dbnode);
	return dbnode;
}

void col_del(Column* col){
	mem_free(col->name_);
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

Column* col_create(int num) {
	Column* col_ = mem_calloc(1,sizeof(Column));
	LIST_INIT(col_);
	col_->data_type = NULL_;
	col_->num_ = num;
	return col_;
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

void table_del(Table * table){
	list_del_all(table->col_head,col_del);
	//LIST_DEL_ALL(table->col_head, col_del);
	mem_free(table->name_);
	mem_free(table);
}





