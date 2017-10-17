#include"Catalog.h"
#include"Mem\MemPool.h"
#include "BaseStruct\Listhead.h"
#include "Meta.h"
#include "StorageEngine\Page.h"
#include <stdio.h>
#include<sys\stat.h>
#include<pthread.h>
#include"StorageEngine\BufferManager.h"

#define NAME_LEN 16 

typedef struct {
	char dbname[NAME_LEN];
	size_t num;
	size_t table_name;
}DatabaseInfo;

typedef struct {
	char tablename[NAME_LEN];
	size_t table_num;
	size_t table_auto_increment;
	size_t table_rec_size;
	size_t able_data_len;
	size_t table_col_count;
	char table_db_name[NAME_LEN];
	size_t table_page_solt_count;
}TableInfo;

typedef struct {
	size_t column_num;
	char column_name[NAME_LEN];
	char column_table_name[NAME_LEN];
	char column_database[NAME_LEN];
	size_t column_data_type;
	size_t column_not_null;
	size_t column_unique;
	size_t column_rec_offset;
}ColunmInfo;

static DBnode sys_database;

static Table sys_database_info;
static Column database_id;//int
static Column database_name;//char[NAME_LEN]

static Table sys_table_info;
static Column table_name;//char[NAME_LEN]
static Column table_num;//int
static Column table_auto_increment;//int
static Column table_rec_size;//int
static Column table_data_len;//int
static Column table_col_count;//int
static Column table_db_name;//char[NAME_LEN]
static Column table_page_solt_count;//int

static Table sys_column_info;
static Column column_num;//int
static Column column_name;//char[NAME_LEN]
static Column column_table_name;//char[NAME_LEN]
static Column column_database;//char[NAME_LEN]
static Column column_data_type;//size_t
static Column column_not_null;//size_t
static Column column_unique;//size_t
static Column column_rec_offset;//size_t

DBnode* read_database_info(void);
int read_table_info(DBnode* db);
int read_column_info(DBnode* db);

Table* sys_read_table(char*);
Column* sys_read_col(char*);

void sys_database_init(DBnode* db, char* name, int id, int tablecount, Table* tablehead);
void sys_table_init(Table* table, char* name,
	int id, int datalen, int columncount,
	int recsize, Column* col, size_t pageslotcount);
void sys_column_init(Column* col, char* name, int id, int datatype, int recoffset);

int database_info_update(DBnode * db);
int col_info_update(Column * col);
int table_info_update(Table * table);

DBnode* read_database_info(void) {
	FILE* fd;
	fclose(fopen("DBInfo.df", "a"));
	fd = fopen("DBInfo.df", "rb+");
	char ch = fgetc(fd);
	int db_count;
	if (ch == EOF) {
		db_count = 0;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fwrite(&db_count, sizeof(int), 1, fd);
		fflush(fd);
		fclose(fd);
	}else {
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fread(&db_count, sizeof(int), 1, fd);
		DBnode *db_ = NULL;
		for (int i = 0; i < db_count; i++) {
			int id_ = 0;
			char* name_ = mem_alloc(20);
			int table_count = 0;
			fread(&id_, sizeof(int), 1, fd);
			fread(name_, sizeof(char), NAME_LEN, fd);
			fread(&table_count, sizeof(int), 1, fd);
			DBnode *next_ = database_create(name_, id_);
			next_->table_count = table_count;
			new_bufferManager(next_);
			if (db_ == NULL) 
				db_ = next_;
			else 
				LIST_ADD_TAIL(&db_->list_head, &next_->list_head);
		}
		fclose(fd);
		return db_;
	}
	return NULL;
}

int read_table_info(DBnode* db) {
	FILE* fd;
	fd = fopen("TableInfo.df", "a");
	fclose(fd);
	fd = fopen("TableInfo.df", "rb+");
	char ch = fgetc(fd);
	Table* table_ = NULL;
	if (ch == EOF) {
		int table_count = 0;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fwrite(&table_count, sizeof(int), 1, fd);
		fflush(fd);
		fclose(fd);
	}else {
		int table_count;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fread(&table_count, sizeof(int), 1, fd);
		for (int i = 0; i < table_count; i++) {
			char table_info[64];
			fread(table_info, 64, 1, fd);
			Table* next_ = sys_read_table(table_info);
			DBnode* db_ = DBNODE_SEARCH(db, next_->db_name);
			bm_add_file_head(db_->id_,
				read_file_head(next_->name_, next_->data_len,
				next_->page_slot_count));
			if (db_->table_head == NULL)
				db_->table_head = next_;
			else
				DB_ADD_TABLE(db_, next_);

		}

	}
	return 0;
}

int read_column_info(DBnode* db) {
	FILE* fd;
	fd = fopen("ColumnInfo.df", "a");
	fclose(fd);
	fd = fopen("ColumnInfo.df", "rb+");
	char ch = fgetc(fd);
	if (ch == EOF) {
		int column_count = 0;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fwrite(&column_count, sizeof(int), 1, fd);
		fflush(fd);
		fclose(fd);
	}else {
		int column_count;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fread(&column_count, sizeof(int), 1, fd);
		for (int i = 0; i < column_count; i++) {
			char column_info[68];
			fread(column_info, 68, 1, fd);
			Column* next_ = sys_read_col(column_info);
			DBnode* db_ = DBNODE_SEARCH(db, next_->db_name);
			Table* table_ = DBNODE_SEARCH(db->table_head, next_->table_name);
			if (table_->col_head == NULL)
				table_->col_head = next_;
			else 
				LIST_ADD_TAIL(&table_->col_head->list_head, &next_->list_head);
		}
	}
	return 0;
}

void sys_database_init(DBnode* db, char* name, int id, int tablecount, Table* tablehead) {
	db->name_ = name;
	db->id_ = id;
	db->table_count = tablecount;
	db->table_head = tablehead;
	LIST_INIT(&db->list_head);
}

void sys_table_init(Table* table, char* name, 
	int id, int datalen, int columncount, 
	int recsize,Column* col,size_t pageslotcount) {

	table->name_ = name;
	table->num_ = id;
	table->data_len = datalen;
	table->column_count = columncount;
	table->rec_size = recsize;
	table->col_head = col;
	table->page_slot_count = pageslotcount;
	LIST_INIT(&table->list_head);
}

void sys_column_init(Column* col, char* name, int id, int datatype, int recoffset) {
	col->name_ = name;
	col->data_type = datatype;
	col->num_ = id;
	col->rec_offset = recoffset;
	LIST_INIT(&col->list_head);
}


int database_info_update(DBnode * db){
	FILE* fd;
	if ((fd = fopen("DBInfo.df", "rb+")) == NULL) {
		fprintf(stderr, "数据库信息文件打开失败\n");
		goto ERROR;
	}
	int db_count;
	fread(&db_count, sizeof(int), 1, fd);
	size_t start_offset = db_count * 20;
	db_count++;
	fseek(fd, 0 - sizeof(int), SEEK_CUR);
	fwrite(&db_count, sizeof(int), 1, fd);
	fseek(fd, start_offset, SEEK_CUR);

	fwrite(&db->id_, sizeof(int), 1, fd);
	fwrite(db->name_, sizeof(char), NAME_LEN, fd);
	fwrite(&db->table_count, sizeof(int), NAME_LEN, fd);

	fflush(fd);
	fclose(fd);
	return 1;
ERROR:
	fclose(fd);
	return 0;
}

int col_info_update(Column * col){
	FILE* fd;
	if ((fd = fopen("ColumnInfo.df", "rb+")) == NULL) {
		fprintf(stderr, "列信息文件打开失败\n");
		goto ERROR;
	}	

	int col_count;
	fread(&col_count, sizeof(int), 1, fd);
	size_t statr_offset = col_count * 68;
	col_count++;
	fseek(fd, 0 - sizeof(int), SEEK_CUR);
	fwrite(&col_count, sizeof(int), 1, fd);

	char col_info[68];
	*(int*)col_info = col->num_;
	int offset_ = sizeof(int);

	strcpy(col_info + offset_, col->name_);
	offset_ += NAME_LEN;

	strcpy(col_info + offset_, col->table_name);
	offset_ += NAME_LEN;

	*(int*)(col_info + offset_) = col->data_type;
	offset_ += sizeof(int);

	*(int*)(col_info + offset_) = col->is_null_able;
	offset_ += sizeof(int);

	*(int*)(col_info + offset_) = col->unique_able;
	offset_ += sizeof(int);

	strcpy(col_info + offset_, col->db_name);
	offset_ += NAME_LEN;

	*(int*)(col_info + offset_) = col->rec_offset;

	fseek(fd, statr_offset, SEEK_CUR);
	fwrite(col_info, 68, 1, fd);

	fflush(fd);
	fclose(fd);
	return 1;
ERROR:
	fclose(fd);
	return 0;
}

Column* sys_read_col(char* colinfo){
	int col_num = *(int*)colinfo;
	int offset_ = sizeof(int);

	int col_name_len = strlen(colinfo + offset_) + 1;
	char* col_name = mem_alloc(col_name_len);
	strcpy(col_name, colinfo + offset_);
	offset_ += NAME_LEN;

	int table_name_len = strlen(colinfo + offset_) + 1;
	char* table_name = mem_alloc(table_name_len);
	strcpy(table_name, colinfo + offset_);
	offset_ += NAME_LEN;

	int data_type = *(int*)(colinfo + offset_);
	offset_ += sizeof(int);

	int is_null_able = *(int*)(colinfo + offset_);
	offset_ += sizeof(int);

	int unique_able = *(int*)(colinfo + offset_);
	offset_ += sizeof(int);

	int db_name_len = strlen(colinfo + offset_);
	char* db_name = mem_alloc(db_name_len);
	strcpy(db_name, colinfo + offset_);
	offset_ += NAME_LEN;

	int rec_offset = *(int*)(colinfo + offset_);

	Column* col_ = col_create(0);
	sys_column_init(col_, col_name, col_num, data_type, rec_offset);
	col_->db_name = db_name;
	col_->table_name = table_name;
	return col_;
}

int table_info_update(Table * table){
	FILE* fd;

	if ((fd = fopen("TableInfo.df", "rb+")) == NULL) {
		fprintf(stderr, "表信息文件打开失败\n");
		goto ERROR;
	}
	int table_count;
	fread(&table_count, sizeof(int), 1, fd);
	size_t start_size = table_count * 64;
	table_count++;
	fseek(fd, 0 - sizeof(int), SEEK_CUR);
	fwrite(&table_count, sizeof(int), 1, fd);

	char table_info[64];
	*(int*)(table_info) = table->num_;
	size_t offset_ = sizeof(int);

	strcpy(table_info + offset_, table->name_);
	offset_ += NAME_LEN;

	*(uint32_t*)(table_info + offset_) = table->auto_increment;
	offset_ += sizeof(uint32_t);

	*(uint32_t*)(table_info + offset_) = table->rec_size;
	offset_ += sizeof(uint32_t);

	*(size_t*)(table_info + offset_) = table->data_len;
	offset_ += sizeof(size_t);
	
	*(int*)(table_info + offset_) = table->column_count;
	offset_ += sizeof(int);

	*(size_t*)(table_info + offset_) = table->page_slot_count;
	offset_ += sizeof(size_t);

	strcpy(table_info + offset_, table->db_name);

	fseek(fd, start_size, SEEK_CUR);
	fwrite(table_info, 64, 1, fd);

	fflush(fd);
	fclose(fd);

	Column *col = NULL;
	LIST_FOREACH(col, table->col_head,
		col_info_update(col);
		);
	return 1;
ERROR:
	fclose(fd);
	return 0;
}

Table* sys_read_table(char* tableinfo) {
	int table_id = *(int*)(tableinfo);
	int offset_ = sizeof(int);

	int table_name_len = strlen(tableinfo + offset_) + 1;
	char* table_name = mem_alloc(table_name_len);
	strcpy(table_name, tableinfo + offset_);
	offset_ += NAME_LEN;

	uint32_t auto_increment = *(uint32_t*)(tableinfo + offset_);
	offset_ += sizeof(int);

	uint32_t rec_size = *(uint32_t *)(tableinfo + offset_);
	offset_ += sizeof(int);

	size_t data_len = *(size_t*)(tableinfo + offset_);
	offset_ += sizeof(int);

	int column_count = *(int*)(tableinfo + offset_);
	offset_ += sizeof(int);

	size_t page_slot_count = *(size_t*)(tableinfo + offset_);
	offset_ += sizeof(size_t);

	int db_name_len = strlen(tableinfo + offset_) + 1;
	char* db_name = mem_alloc(db_name_len);
	strcpy(db_name, tableinfo + offset_);

	Table* table_ = table_create();
	sys_table_init(table_, table_name, table_id, data_len, 
		column_count, rec_size, NULL, page_slot_count);
	table_->db_name = db_name;
	return table_;
}

DBnode* init_sys_data() {


	sys_database_init(&sys_database, "SysDataBase", 0, 4, &sys_database_info);

	sys_table_init(&sys_database_info, "DataBaseInfoD", 0, 20, 2, 0, &database_id, 0);
	sys_column_init(&database_id, "DataBaseId", 0, INT, 0);
	sys_column_init(&database_name, "DataBaseName", 1, CHAR, 4);
	LIST_ADD_TAIL(&sys_database_info.list_head, &sys_table_info.list_head);
	LIST_ADD_TAIL(&sys_database_info.list_head, &sys_column_info.list_head);

	sys_table_init(&sys_table_info, "TableInfo", 1, 52, 6, 0, &table_num, 0);
	sys_column_init(&table_num, "TableId", 0, INT, 0);
	sys_column_init(&table_name, "TableName", 1, CHAR, sizeof(int));
	sys_column_init(&table_auto_increment, "Auto_Increment", 2, INT, sizeof(int) + NAME_LEN);
	sys_column_init(&table_rec_size, "RecCount", 3, INT, 2 * sizeof(int) + NAME_LEN);
	sys_column_init(&table_data_len, "DataLen", 4, INT, 3 * sizeof(int) + NAME_LEN);
	sys_column_init(&table_col_count, "ColumnCount", 5, INT, 4 * sizeof(int) + NAME_LEN);
	sys_column_init(&table_db_name, "DataBaseName", 6, CHAR, 5 * sizeof(int) + NAME_LEN);
	LIST_ADD_TAIL(&table_num.list_head, &table_name.list_head);
	LIST_ADD_TAIL(&table_num.list_head, &table_auto_increment.list_head);
	LIST_ADD_TAIL(&table_num.list_head, &table_rec_size.list_head);
	LIST_ADD_TAIL(&table_num.list_head, &table_data_len.list_head);
	LIST_ADD_TAIL(&table_num.list_head, &table_col_count.list_head);
	LIST_ADD_TAIL(&table_num.list_head, &table_db_name.list_head);

	sys_table_init(&sys_column_info, "ColumnInfo", 2, 68, 6, 0, &column_num, 0);
	sys_column_init(&column_num, "ColumnId", 0, INT, 0);
	sys_column_init(&column_name, "ColumnName", 1, CHAR, sizeof(int));
	sys_column_init(&column_table_name, "TableName", 2, CHAR, sizeof(int) + NAME_LEN);
	sys_column_init(&column_data_type, "DataType", 3, INT, sizeof(int) + 32);
	sys_column_init(&column_not_null, "IsNullAble", 4, INT, 2 * sizeof(int) + 32);
	sys_column_init(&column_unique, "IsUniqueAble", 5, INT, 3 * sizeof(int) + 32);
	sys_column_init(&column_database, "DataBaseName", 6, CHAR, 4 * sizeof(int) + 32);
	sys_column_init(&column_rec_offset, "RecOffset", 7, INT, 4 * sizeof(int) + 48);

	LIST_ADD_TAIL(&column_num.list_head, &column_name.list_head);
	LIST_ADD_TAIL(&column_num.list_head, &column_table_name.list_head);
	LIST_ADD_TAIL(&column_num.list_head, &column_data_type.list_head);
	LIST_ADD_TAIL(&column_num.list_head, &column_not_null.list_head);
	LIST_ADD_TAIL(&column_num.list_head, &column_unique.list_head);
	LIST_ADD_TAIL(&column_num.list_head, &column_database.list_head);
	DBnode* db = read_database_info();
	read_table_info(db);
	read_column_info(db);
	DBnode* sys = &sys_database;
	if (db) {
		db->list_head.prve_->next_ = sys;
		sys->list_head.prve_ = db->list_head.prve_;
		sys->list_head.next_ = db;
		db->list_head.prve_ = sys;
	}
	return sys;
}

void LDB_init(){
	FILE* fd;
	fclose(fopen("DBInfo.df", "a"));
	fd = fopen("DBInfo.df", "rb+");
	char ch = fgetc(fd);
	int db_count;
	if (ch == EOF) {
		db_count = 0;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fwrite(&db_count, sizeof(int), 1, fd);
		fflush(fd);
		fclose(fd);
	}else {
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fread(&db_count, sizeof(int), 1, fd);
		DBnode *db_ = NULL;
		for (int i = 0; i < db_count; i++) {
			int id_ = 0;
			char* name_ = mem_alloc(20);
			int table_count = 0;
			fread(&id_, sizeof(int), 1, fd);
			fread(name_, sizeof(char), NAME_LEN, fd);
			fread(&table_count, sizeof(int), 1, fd);
			DBnode *next_ = database_create(name_, id_);
			next_->table_count = table_count;
			new_bufferManager(next_);
			if (db_ == NULL) 
				db_ = next_;
			else 
				LIST_ADD_TAIL(&db_->list_head, &next_->list_head);
		}
		fclose(fd);
		return db_;
	}
	return NULL;
}
}




