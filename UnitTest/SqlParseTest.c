#include"TestFrameWork.h"
#include"../QueryProcessor/Sqlparse.h"
#include"../Meta.h"
#include"../StorageEngine/BufferManager.h"
static char sql_db[64] = "create database db1";
static char sql_table[64] = "create table t1(col1 int, col2 char, col3 char(10),col4 float)";
static char sql_insert1[64] = "insert into test_table values(1,'col3',1.22)";
static char sql_insert2[64] = "insert into test_table values(1,'col3')";
static char sql_insert3[64] = "insert into test_table values(1,'col3',1.22,10)";
static char sql_insert4[64] = "insert into test_table(col1,col3,col4) values(1,'col3',1.22)";
static char sql_insert5[64] = "insert into test_table(col1,col2) values(1,'col3',1.22)";
static char sql_from1[64] = "select col1,test_table.col2,col3 from test_table";
static char* p_sql_from1 = sql_from1;
static char* p_insert1 = sql_insert1;
static char* p_insert2 = sql_insert2;
static char* p_insert3 = sql_insert3;
static char* p_insert4 = sql_insert4;
static char* p_insert5 = sql_insert5;
static char* p_db = sql_db;
static char* p_table = sql_table;

static char errmsg[128];

static DBnode* test_db;
static Table* t;
static Table* t2;

static Column* col1;
static Column* col2;
static Column* col3;
static Column* col4;
static Column* col5;
static Column* col6;

void sql_test_init(void) {
	test_db = database_create("test_db", 0, 0);
	t = new_table("test_table", "test", 0);
	col1 = new_column("col1", "test_table", "test_db", 0, INT, 4);
	col2 = new_column("col2", "test_table", "test_db", 1, CHAR, 16);
	col3 = new_column("col3", "test_table", "test_db", 2, FLOAT, 4);
	table_add_col(t, col1);
	table_add_col(t, col2);
	table_add_col(t, col3);
	db_add_table(test_db, t);

	t2 = new_table("test_table2", "test", 0);
	col4 = new_column("col1", "test_table2", "test_db", 0, INT, 4);
	col5 = new_column("col2", "test_table2", "test_db", 1, CHAR, 16);
	col6 = new_column("col3", "test_table2", "test_db", 2, FLOAT, 4);
	table_add_col(t2, col4);
	table_add_col(t2, col5);
	table_add_col(t2, col6);
	db_add_table(test_db, t2);

	new_bufferManager(test_db);
	FHead* f =	new_file_head("test_table", 
			new_file_head_data(PageCount, 16, 24));
	bm_add_raw_file_head(get_buffman(0), f);
	init_file(f);

	init_key_word();
}

static void c_db_test(void) {
	int cnum = 0;
	int lnum = 0;
	Token* t = scanner(errmsg, &p_db, &cnum, &lnum);
	sql_parse(errmsg, test_db, t, NULL);
	DBnode *d = DBNODE_SEARCH(test_db, "db1");
	EXPECT_EQ_STR("db1", d->name_);
}

static void c_table_test(void) {
	int cnum = 0;
	int lnum = 0;
	Token* t = scanner(errmsg, &p_table, &cnum, &lnum);
	sql_parse(errmsg, test_db, t, NULL);
	Table* tab = db_get_table(test_db, "t1");
	EXPECT_EQ_STR("t1", tab->t_info.table_name);
	remove("t1");
}

static void insert_test(void) {
	int cnum = 0;
	int lnum = 0;
	QueryNode* node = NULL;
	QueryNode* node2 = NULL;
	QueryNode* node3 = NULL;
	QueryNode* node4 = NULL;
	QueryNode* node5 = NULL;
	Token* t = scanner(errmsg, &p_insert1, &cnum, &lnum);
	Token* t1 = scanner(errmsg, &p_insert2, &cnum, &lnum);
	Token* t2 = scanner(errmsg, &p_insert3, &cnum, &lnum);
	Token* t3 = scanner(errmsg, &p_insert4, &cnum, &lnum);
	Token* t5 = scanner(errmsg, &p_insert5, &cnum, &lnum);

	sql_parse(errmsg, test_db, t, &node);
	EXPECT_EQ_INT(INSERT, node->type);
	char* row = node->insert_node->insert_row;
	EXPECT_EQ_INT(NULL, row == NULL);
	EXPECT_EQ_INT(1, *(int*)row);
	EXPECT_EQ_STR("col3", row + sizeof(int));

	sql_parse(errmsg, test_db, t1, &node2);
	puts(errmsg);

	sql_parse(errmsg, test_db, t2, &node3);
	puts(errmsg);

	sql_parse(errmsg, test_db, t3, &node4);
	puts(errmsg);

	sql_parse(errmsg, test_db, t5, &node5);
	puts(errmsg);

	//EXPECT_EQ_INT(INSERT, node4->type);
	//EXPECT_EQ_INT(NULL, node4->insert_node);
	//EXPECT_EQ_INT(1, node4->insert_rows.usedsize_);
	//row = VECTOR_GET_VALUE(&node4->insert_rows, 0);
	//EXPECT_EQ_INT(1, *(int*)row);
	//EXPECT_EQ_STR("col3", row + sizeof(int));

	//free node
}

static from_test(void) {
	static char sql[100] = "select * from test_table,test_table2";
	static char* p_ = sql;
	int cnum = 0;
	int lnum = 0;
	int c = 0, l = 0;
	QueryNode* node1 = NULL;
	QueryNode* node2 = NULL;
	QueryNode* node3 = NULL;
	QueryNode* node4 = NULL;
	Token* t1 = scanner(errmsg, &p_sql_from1, &cnum, &lnum);
	Token* t2 = scanner(errmsg, &p_, &c, &l);

	sql_parse(errmsg, test_db, t1, &node1);

	EXPECT_EQ_INT(SELECT, node1->type);
	
	EXPECT_EQ_STR("test_table",
	node1->select_node->from_items->table_->t_info.table_name);

	EXPECT_EQ_STR("col1",
	node1->select_node->select_items->col_->column_name);

	DBitems* next = node1->select_node->select_items->head.next_;

	EXPECT_EQ_STR("col2", next->col_->column_name);
	EXPECT_EQ_STR("test_table", 
		next->table_->t_info.table_name);

	sql_parse(errmsg, test_db, t2, &node2);

	EXPECT_EQ_STR("test_table",node2->select_node->join->
		left->table_->t_info.table_name);
	EXPECT_EQ_STR("test_table2",node2->select_node->join->
		right->table_->t_info.table_name);
	//puts(errmsg);
}

void where_test() {
	static char sql[100] = "select* from test_table,test_table2 where col1 = test_table2.col2";
	static char* p_ = sql;
	int cnum = 0, lnum = 0;

	QueryNode* node1 = NULL;
	Token* t1 = scanner(errmsg, &p_, &cnum, &lnum);
	sql_parse(errmsg, test_db, t1, &node1);
	puts(errmsg);

	//EXPECT_EQ_INT(EQUAL, 
	//	node1->select_node->condition->operator_);
	//EXPECT_EQ_INT(0, 
	//	node1->select_node->condition->res_type);
	//EXPECT_EQ_STR("test_table",
	//	node1->select_node->condition->left->opand->table_->t_info.table_name);
	//EXPECT_EQ_STR("col1",
	//	node1->select_node->condition->left->opand->col_->column_name);
}

void select_test() {
	
}

void sqltest(void) {
	sql_test_init();
	c_db_test();
	c_table_test();
	insert_test();
	from_test();
	//where_test();
	select_test();
	//remove("test_table");
}