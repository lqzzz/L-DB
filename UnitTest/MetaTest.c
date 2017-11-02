//#include"../Meta.c"
#include"../Meta.h"
#include"TestFrameWork.h"

static DBnode* dbhead;

static DBnode* sysdb;
static DBnode* db1;
static DBnode* db2;
static Table* t11;
static Table* t12;
static Table* t21;
static Table* t22;
static Column* col1;
static Column* col2;
static Column* col3;
static Column* col4;
static Column* col5;
static Column* col6;
static Column* col7;
static Column* col8;

int database_info_update(DBnode * db);
int col_info_update(Column * col);
int table_info_update(Table * table);

static void meta_test_init(void) {
	db1 = database_create("T_1", 0, 0);
	db2 = database_create("T_2", 1, 0);
	LIST_ADD_TAIL(&db1->list_head, &db2->list_head);

	t11 = new_table("T_1_1", "T_1", 0);
	t12 = new_table("T_1_2", "T_1", 1);
	t21 = new_table("T_2_1", "T_2", 0);
	t22 = new_table("T_2_2", "T_2", 1);
	col1 = new_column("T_1_1_1", "T_1_1", "T_1", 0, INT, 4);
	col2 = new_column("T_1_1_2", "T_1_1", "T_1", 1, CHAR, 16);
	col3 = new_column("T_1_2_1", "T_1_2", "T_1", 0, INT, 4);
	col4 = new_column("T_1_2_2", "T_1_2", "T_1", 1, CHAR, 16);
	col5 = new_column("T_2_1_1", "T_2_1", "T_2", 0, INT, 4);
	col6 = new_column("T_2_1_2", "T_2_1", "T_2", 1, CHAR, 16);
	col7 = new_column("T_2_2_1", "T_2_2", "T_2", 0, INT, 4);
	col8 = new_column("T_2_2_2", "T_2_2", "T_2", 1, CHAR, 16);

	table_add_col(t11, col1);
	table_add_col(t11, col2);
	table_add_col(t12, col3);
	table_add_col(t12, col4);
	table_add_col(t21, col5);
	table_add_col(t21, col6);
	table_add_col(t22, col7);
	table_add_col(t22, col8);

	db_add_table(db1, t11);
	db_add_table(db1, t12);
	db_add_table(db2, t21);
	db_add_table(db2, t22);

}

static void add_test(void) {

	EXPECT_EQ_INT(2, db1->table_count);
	EXPECT_EQ_INT(2, db2->table_count);

	EXPECT_EQ_INT(2, t11->t_info.table_col_count);
	EXPECT_EQ_INT(2, t12->t_info.table_col_count);
	EXPECT_EQ_INT(2, t21->t_info.table_col_count);
	EXPECT_EQ_INT(2, t22->t_info.table_col_count);

	EXPECT_EQ_INT(0, col1->column_rec_offset);
	EXPECT_EQ_INT(4, col2->column_rec_offset);
	EXPECT_EQ_INT(0, col3->column_rec_offset);
	EXPECT_EQ_INT(4, col4->column_rec_offset);
	EXPECT_EQ_INT(0, col5->column_rec_offset);
	EXPECT_EQ_INT(4, col6->column_rec_offset);
	EXPECT_EQ_INT(0, col7->column_rec_offset);
	EXPECT_EQ_INT(4, col8->column_rec_offset);
}

static void read_test(void) {
	dbhead = init_sys_data();
	EXPECT_EQ_INT(NULL, dbhead);

	database_info_update(db1);
	dbhead = init_sys_data();
	EXPECT_EQ_INT(0, dbhead->id_);

	table_info_update(t11);
	table_info_update(t12);
	table_info_update(t21);
	table_info_update(t22);

	col_info_update(col1);
	col_info_update(col2);
	col_info_update(col3);
	col_info_update(col4);
	col_info_update(col5);
	col_info_update(col6);
	col_info_update(col7);
	col_info_update(col8);

	db_print(dbhead);
}

void meta_test(void) {
	meta_test_init();
	//add_test();
	//db_print(db1);
	read_test();
	remove("ColumnInfo.df");
	remove("TableInfo.df");
	remove("DBInfo.df");
}
