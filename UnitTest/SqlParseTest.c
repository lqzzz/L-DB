#include"TestFrameWork.h"
#include"../QueryProcessor/Sqlparse.h"
#include"../Meta.h"
static char* db = "create database db1";
static char* use = "use db1";
static char* table = "create table t1(col1 int, col2 char, col3 char(10))";
static char sql_db[64];
static char sql_use[32];
static char sql_table[100];
static char errmsg[100];
static DBnode* test_db;

void sql_test_init(void) {
	test_db = database_create("test", 0, 0);
	init_key_word();
	strcpy(sql_db, db);
	strcpy(sql_use, use);
	strcpy(sql_table, table);
}

static void c_db_test(void) {
	int cnum = 0;
	int lnum = 0;
	char* s = sql_db;
	Token* t = scanner(errmsg, &s, &cnum, &lnum);
	sql_parse(errmsg, test_db, t);
	DBnode *d = DBNODE_SEARCH(test_db, "db1");
	EXPECT_EQ_STR("db1", d->name_);
}

static void c_table_test(void) {
	int cnum = 0;
	int lnum = 0;
	char* s = sql_table;
	Token* t = scanner(errmsg, &s, &cnum, &lnum);
	sql_parse(errmsg, test_db, t);
	Table* tab = db_get_table(test_db, "t1");
	EXPECT_EQ_STR("ti", tab->t_info.table_name);

}

void sqltest(void) {
	sql_test_init();
	//c_db_test();
	c_table_test();
}