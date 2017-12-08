#include"Catalog.h"
#include"Mem\MemPool.h"
#include "BaseStruct\Listhead.h"
#include "Meta.h"
#include "StorageEngine\Page.h"
#include <stdio.h>
#include<sys\stat.h>
#include"StorageEngine\BufferManager.h"
typedef struct {
	size_t db_num;
	char dbname[NAME_LEN];
	size_t table_count;
}DatabaseInfo;
static DBnode* dbhead = NULL;

static DBnode sys_database;

static Table sys_database_info;
static Column database_id;
static Column database_name;
static Column database_table_count;

static Table sys_table_info;
static Column table_name;
static Column table_num;
static Column table_auto_increment;
static Column table_rec_size;
static Column table_data_len;
static Column table_col_count;
static Column table_db_name;
static Column table_page_slot_count;

static Table sys_column_info;
static Column column_num;
static Column column_name;
static Column column_data_type;
static Column column_not_null;
static Column column_unique;
static Column column_rec_offset;
static Column column_table_name;
static Column column_database_name;

int read_database_info(void);
int read_table_info(void);
int read_column_info(void);

int database_info_update(DBnode * db);
int col_info_update(Column * col);
int table_info_update(Table * table);

int read_database_info(void) {
	FILE* fd;
	fclose(fopen("DBInfo.df", "a"));
	fd = fopen("DBInfo.df", "rb+");
	char ch = fgetc(fd);
	int db_count;
	if (ch == EOF) {
		db_count = 0;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fwrite(&db_count, sizeof(size_t), 1, fd);
		fflush(fd);
		fclose(fd);
		return 0;
	}else {
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fread(&db_count, sizeof(size_t), 1, fd);
		DatabaseInfo info;
		for (int i = 0; i < db_count; i++) {
			fread(&info, sizeof(DatabaseInfo), 1, fd);
			DBnode *next_ = database_create(info.dbname, 
				info.db_num, info.table_count);
			new_buffermanager(next_->id_);
			if (dbhead == NULL)
				dbhead = next_;
			else LIST_ADD_TAIL(&dbhead->list_head, &next_->list_head);
		}
		fclose(fd);
		return db_count;
	}
}

int read_table_info(void) {
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
		return 0;
	}else {
		int table_count;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fread(&table_count, sizeof(int), 1, fd);
		for (int i = 0; i < table_count; i++) {
			Table* t = new_table("", "", 0);
			fread(&t->t_info, sizeof(TableInfo), 1, fd);
			TableInfo *info = &t->t_info;
			DBnode* db_ = DBNODE_SEARCH(dbhead, info->table_db_name);

			bm_add_raw_file_head(get_buffman(db_->id_),
				read_file_head(info->table_name));

			db_add_table(db_, t);
		}
		fclose(fd);
		return table_count;
	}
}

int read_column_info(void) {
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
		return 0;
	}else {
		int column_count;
		fseek(fd, 0 - sizeof(char), SEEK_CUR);
		fread(&column_count, sizeof(int), 1, fd);
		for (int i = 0; i < column_count; i++) {

			Column* col_ = new_column(NULL, NULL, NULL, 0, 0, 0);
			fread(&col_, sizeof(ColumnInfo), 1, fd);

			DBnode* db_ = DBNODE_SEARCH(&dbhead->list_head, col_->column_db_name);

			Table* table_ = db_get_table(db_, col_->column_table_name);

			table_add_col(table_, col_);
		}
		fclose(fd);
		return column_count;
	}
}

int database_info_update(DBnode * db){
	FILE* fd;
	if ((fd = fopen("DBInfo.df", "rb+")) == NULL) {
		fprintf(stderr, "数据库信息文件打开失败\n");
		return 0;
	}

	int db_count;
	fread(&db_count, sizeof(size_t), 1, fd);
	size_t offset = db_count * sizeof(DatabaseInfo);
	db_count++;
	fseek(fd, 0 - sizeof(size_t), SEEK_CUR);
	fwrite(&db_count, sizeof(size_t), 1, fd);
	fseek(fd, offset, SEEK_CUR);

	fwrite(&db->id_, sizeof(size_t), 1, fd);
	fwrite(db->name_, sizeof(char), NAME_LEN, fd);
	fwrite(&db->table_count, sizeof(size_t), NAME_LEN, fd);

	fflush(fd);
	fclose(fd);
	return 1;
}

int col_info_update(Column * col){
	FILE* fd;
	if ((fd = fopen("ColumnInfo.df", "rb+")) == NULL) {
		fprintf(stderr, "列信息文件打开失败\n");
		return 0;
	}	

	size_t col_count;
	fread(&col_count, sizeof(size_t), 1, fd);
	size_t statr_offset = col_count * sizeof(ColumnInfo);
	col_count++;
	fseek(fd, 0 - sizeof(size_t), SEEK_CUR);
	fwrite(&col_count, sizeof(size_t), 1, fd);
	fseek(fd, statr_offset, SEEK_CUR);

	fwrite(&col, sizeof(ColumnInfo), 1, fd);

	fflush(fd);
	fclose(fd);
	return 1;
}

int table_info_update(Table * table){
	FILE* fd;

	if ((fd = fopen("TableInfo.df", "rb+")) == NULL) {
		fprintf(stderr, "表信息文件打开失败\n");
		return 0;
	}

	size_t table_count;
	fread(&table_count, sizeof(size_t), 1, fd);
	size_t start_size = table_count * 64;
	table_count++;
	fseek(fd, 0 - sizeof(size_t), SEEK_CUR);
	fwrite(&table_count, sizeof(size_t), 1, fd);
	fseek(fd, start_size, SEEK_CUR);

	fwrite(&table->t_info, sizeof(TableInfo), 1, fd);

	fflush(fd);
	fclose(fd);

	return 1;
}

DBnode* init_sys_data() {

	////初始化数据库信息表
	//sys_database.id_ = 0;
	//sys_database.table_count = 0;
	//LIST_INIT(&sys_database.list_head);
	//strcpy(sys_database.name_, "Sys_database");
	//VECTOR_INIT_LEN(&sys_database.tables, 4);

	//db_add_table(&sys_database, &sys_database_info);
	//db_add_table(&sys_database, &sys_table_info);
	//db_add_table(&sys_database, &sys_column_info);

	//table_init(&sys_database_info, "DataBaseInfo", 0, sizeof(DatabaseInfo), 0, 0, 0);
	//table_add_col(&sys_database_info, &database_id);
	//table_add_col(&sys_database_info, &database_name);
	//table_add_col(&sys_database_info, &database_table_count);

	//col_set_info(&database_id, 0, INT, 1, 1, 0, 0, 0,0, "DataBaseId", "DataBaseInfo", "Sys_database");
	//col_set_info(&database_name, 1, CHAR, 1, 1, 0, 0, 0,0, "DataBaseId", "DataBaseInfo", "Sys_database");
	//col_set_info(&database_table_count, 2, INT, 1, 1, 0, 0, 0,0, "DataBaseId", "DataBaseInfo", "Sys_database");

	////初始化表信息表
	//table_init(&sys_table_info, "TableInfo", 1, sizeof(TableInfo), 0, 0, 0);
	//col_set_info(&table_num, 0, INT, 0, 0, 0, 0, "TableId", "TableInfo", "Sys_database");
	//col_set_info(&table_name, 1, CHAR, 0, 0, 0, 0, "TableName", "TableInfo", "Sys_database");
	//col_set_info(&table_auto_increment, 2, INT, 0, 0, 0, 0, "TableAutoIncrement", "TableInfo", "Sys_database");
	//col_set_info(&table_rec_size, 3, INT, 0, 0, 0, 0, "TableRecSize", "TableInfo", "Sys_database");
	//col_set_info(&table_data_len, 4, INT, 0, 0, 0, 0, "TableData_Len", "TableInfo", "Sys_database");
	//col_set_info(&table_col_count, 5, INT, 0, 0, 0, 0, "TableColCount", "TableInfo", "Sys_database");
	//col_set_info(&table_db_name, 6, CHAR, 0, 0, 0, 0, "TableDbName", "TableInfo", "Sys_database");
	//col_set_info(&table_page_slot_count, 7, INT, 0, 0, 0, 0, "TablePageSlotCount", "TableInfo", "Sys_database");
	//table_add_col(&sys_table_info, &table_num);
	//table_add_col(&sys_table_info, &table_name);
	//table_add_col(&sys_table_info, &table_auto_increment);
	//table_add_col(&sys_table_info, &table_rec_size);
	//table_add_col(&sys_table_info, &table_data_len);
	//table_add_col(&sys_table_info, &table_col_count);
	//table_add_col(&sys_table_info, &table_db_name);
	//table_add_col(&sys_table_info, &table_page_slot_count);

	////初始化列信息表
	//table_init(&sys_column_info, "ColumnInfo", 2, sizeof(ColumnInfo), 0, 0, 0);
	//col_set_info(&column_num, 0, INT, 0, 0, 0,0, "ColumnId", "ColumnInfo", "Sys_database");
	//col_set_info(&column_name, 1, CHAR, 0, 0, 0, 0, "ColumnName", "ColumnInfo", "Sys_database");
	//col_set_info(&column_data_type, 2, CHAR, 0, 0, 0, 0, "ColumnDataType", "ColumnInfo", "Sys_database");
	//col_set_info(&column_not_null, 3, INT, 0, 0, 0, 0, "ColumnNotNull", "ColumnInfo", "Sys_database");
	//col_set_info(&column_unique, 4, INT, 0, 0, 0, 0, "ColumnUnique", "ColumnInfo", "Sys_database");
	//col_set_info(&column_rec_offset, 5, INT, 0, 0, 0, 0, "ColumnRecOffset", "ColumnInfo", "Sys_database");
	//col_set_info(&column_table_name, 6, CHAR, 0, 0, 0, 0, "ColumnTableName", "ColumnInfo", "Sys_database");
	//col_set_info(&column_database_name, 7, CHAR, 0, 0, 0, 0, "ColumnDatabaseName", "ColumnInfo", "Sys_database");
	//table_add_col(&sys_column_info, &column_num);
	//table_add_col(&sys_column_info, &column_name);
	//table_add_col(&sys_column_info, &column_data_type);
	//table_add_col(&sys_column_info, &column_not_null);
	//table_add_col(&sys_column_info, &column_unique);
	//table_add_col(&sys_column_info, &column_rec_offset);
	//table_add_col(&sys_column_info, &column_table_name);
	//table_add_col(&sys_column_info, &column_database_name);

	if (read_database_info() != 0) {
		read_table_info();
		read_column_info();
	}

	return dbhead;
}

void db_show(char * name){
	
}

void table_show(DBnode * db, char * name){
	

}

void col_show(Table * t, char * name){
	
}





