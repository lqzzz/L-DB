#ifndef _DB_H
#define _DB_H
#include "listhead.h"
#include "Vector.h"
#include "Page.h"
//#include<WinSock2.h>
#define INIT_RECROD_NUM 16 

#define DBNODE_SEARCH(head,key) list_search(head,key,db_match_name)
#define DB_ADD_TABLE(db,table) LIST_ADD_TAIL(&(db)->table_head->list_head, &(table)->list_head); (db)->table_count++

#define REC_GET_DATA(rec) (rec)->data_	
#define REC_FILL(rec,offset,data,datalen) memcpy((char*)((Record*)(rec))->data_ + offset,data,datalen)

#define TABLE_ADD_REC(table,rec) VECTOR_PUSHBACK(&(table)->rec_vec, rec)
#define TABLE_GET_REC_SIZE(table) VECTOR_GET_SIZE(&(table)->rec_vec) 
#define TABLE_USED_REC_SIZE(table) (table)->rec_size
#define TABLE_GET_REC_VEC(table) &(table)->rec_vec
#define TABLE_GET_REC(table,index) VECTOR_GET_VALUE(TABLE_GET_REC_VEC(table),index)
#define TABLE_GET_COL_SIZE(table) table->column_count
#define TABLE_GET_COL_HEAD(table) table->col_head
#define TABLE_FILL_REC(table,rec_index,offset,data,datalen) REC_FILL(TABLE_GET_REC(table,rec_index),offset,data,datalen)

enum Tokentype {
	CREATE = 1, USE, DATABASE, TABLE, INDEX, UPDATE, PROCEDURE, VIEW,
	NUMBER, TEXT, REAL, DATE, DATETIME, TIME, INT, CHAR, BOOL, VARCHAR,
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
	int col_num;
	int row_num;
	size_t col_set_len;
	size_t row_set_len;
	
	char buf_[];//col_set and row_set or msg
}Sendset;

typedef struct column {
	Listhead list_head; 
	char* name_;
	int num_;
	size_t data_len;
	size_t rec_offset;
	size_t max_data_len;
	char* table_name;
	char* db_name;
	enum Tokentype data_type;
	struct column* fk_;
	int is_null_able;
	int unique_able;
}Column;

typedef struct Record {
	char* schema_name;
	char* time_stamp;
	void* data_;
}Record;

typedef struct Table {
	Listhead list_head;
	char* name_;
	int num_;
	Column *col_head; //collist
	uint32_t auto_increment;
	uint32_t rec_size;
	size_t data_len;
	int column_count;
	size_t page_slot_count;
	char* db_name;
}Table;

typedef struct DBnode {
	Listhead list_head;
	char *name_;
	int id_;
	Table* table_head;//tablelist
	int table_count;
}DBnode;

typedef struct {
	DBnode database_;
}CataLog;

DBnode* init_sys_data();

void database_del(DBnode* db);
DBnode* database_create(char* name,int id);
int database_info_update(DBnode* db);

int db_match_name(DBnode* node, const char* name);
void* dbnode_search(void* head, const char* name);

Table* table_create();
void table_del(Table* table);
int table_info_update(Table* table);

Column* col_create(int num);
void col_del(Column* col);
int col_info_update(Column* col);
size_t get_max_data_len(void* item,enum Tokentype datatype,size_t currmaxlen);

Record* rec_create(char* schemaname, size_t len);
void* rec_dup_data(Record* rec,size_t datalen);
void rec_del(Record* rec);

#endif // !_DB_H

