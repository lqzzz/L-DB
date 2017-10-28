#ifndef _CATALOG_H
#define _CATALOG_H
#include "BaseStruct/Listhead.h"
#include "BaseStruct/Vector.h"

#define NAME_LEN 24 

#define DBNODE_SEARCH(head,key) list_search(head,key,db_match_name)

enum Tokentype {
	CREATE = 1, USE, DATABASE, TABLE, INDEX, UPDATE, 
	NUMBER, TEXT, FLOAT, DATE, DATETIME, TIME, INT, CHAR, BOOL, VARCHAR,
	PRIMARY, KEY, FOREIGN, REFERENCES, NULL_, DEFAULT, UNIQUE,
	LIKE, AND, BETWEEN, OR, IN, EXISTS, DOT,
	NOT,NOT_IN,NOT_EXISTS,
	SELECT, DISTINCT, WHERE, FROM, GROUP, BY, ORDER, HAVING,
	ALTER, ADD, DROP, INSERT,
	COMMIT, TRANSACTION, ROLLBACK,
	INTO, VALUES, ON, SET,
	ALL, AS, DELETE, UNION,
	AVG, MAX, MIN, COUNT,
	ID, SYMBOL,LB, RB, COMMA, SQM, SEM, STAR,
	LESSTHAN, GREATERTHAN, GREATER_OR_EQ, LESS_OR_EQ, EQUAL,NOT_EQUAL,
	JOIN
};

typedef struct {
	size_t table_num;
	size_t table_auto_increment;
	size_t table_rec_size;
	size_t table_data_len;
	size_t table_col_count;
	size_t table_page_solt_count;
	char table_db_name[NAME_LEN];
	char table_name[NAME_LEN];
}TableInfo;

typedef struct {
	size_t column_num;
	enum TokenType column_data_type;
	size_t column_not_null;
	size_t column_unique;
	size_t column_rec_offset;
	size_t column_data_len;
	char column_name[NAME_LEN];
	char column_table_name[NAME_LEN];
	char column_db_name[NAME_LEN];
}ColumnInfo;

typedef struct Record {
	char* schema_name;
	char* time_stamp;
	void* data_;
}Record;

typedef struct Table {
	TableInfo t_info;
	Vector cols;
	Vector memrows;
}Table;

typedef struct DBnode {
	Listhead list_head;
	char name_[NAME_LEN];
	int id_;
	int table_count;
	Vector tables;
}DBnode;

typedef ColumnInfo Column;

void database_del(DBnode* db);
DBnode* database_create(char* name, size_t id, size_t tablecount);
int db_match_name(DBnode* node, const char* name);
void* dbnode_search(void* head, const char* name);
Table* db_get_table(DBnode* db, char* tablename);
void db_add_table(DBnode* db, Table* t);
void db_print(DBnode* dbh);

void table_print(DBnode* db);
Table* new_table(char* tablename,char* dbname,size_t id);
void table_del(Table* table);
int table_cmp_name(Table* t1, Table* t2);
Column* table_get_col(Table* t, char* colname);
void table_add_col(Table* t, Column* col);
void table_init(Table* table, char* name,
	size_t id, size_t datalen,
	size_t columncount,size_t recsize,
	size_t pageslotcount);
//void table_print(Table* t);

Column* new_column(char* colname, char* tablename, char*dbname, size_t id,enum TokenType DT,size_t datalen);
int col_cmp_name(Column* col1,Column* col2);
void col_del(Column* col);
size_t get_max_data_len(void* item,enum Tokentype datatype,size_t currmaxlen);
void col_set_info(Column* col,
	size_t column_num,enum TokenType column_data_type,
	size_t column_not_null,size_t column_unique,
	size_t column_rec_offset,size_t data_len,char* column_name,
	char* column_table_name,char* column_db_name );

Record* rec_create(char* schemaname, size_t len);
void* rec_dup_data(Record* rec,size_t datalen);
void rec_del(Record* rec);

#endif // !_CATALOG_H

