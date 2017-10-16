#include"../BaseStruct/dict.h"
#include"../BaseStruct/Vector.h"
#include"../BaseStruct/Listhead.h"
#include"../StorageEngine/Page.h"
#include<stdio.h>
static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_STR(expect, actual) EXPECT_EQ_BASE(dictstrcmp((expect),(actual)), expect, actual, "%d")

int dictstrcmp(const char* str1, const char* str2) {
	return strcmp(str1, str2) == 0 ? 1 : 0;
}

dict_print_arr(Dict* d) {
	printf("hasharray : ");
	for (size_t i = 0; i < d->ht_.size_; i++){
		int num = 0;
		for (DictEntry* e = d->ht_.table_[i]; e; num++, e = e->next_);
		printf("%d ", num);
	}
	printf("\n");
}

void dict_test_add(Dict* d) {
	printf("--------dict_test_add-------- \n");
	dict_add_entry(d, "key1", 1);
	EXPECT_EQ_INT(DICT_HT_INITIAL_SIZE, d->ht_.size_);
	EXPECT_EQ_INT(1, d->ht_.used_);
	dict_add_entry(d, "key2", 2);
	dict_add_entry(d, "key3", 3);
	dict_add_entry(d, "key4", 4);
	dict_print_arr(d);
	dict_add_entry(d, "key5", 5);
	dict_add_entry(d, "key6", 6);
	dict_add_entry(d, "key7", 7);
	dict_print_arr(d);
	dict_add_entry(d, "key8", 8);
	dict_add_entry(d, "key9", 9);
	dict_add_entry(d, "key10", 10);
	dict_print_arr(d);
	EXPECT_EQ_INT(10, d->ht_.used_);
	EXPECT_EQ_INT(8, d->ht_.size_);
	EXPECT_EQ_INT(dict_get_value(d, "key1"), 1);
	EXPECT_EQ_INT(dict_get_value(d, "key2"), 2);
	EXPECT_EQ_INT(dict_get_value(d, "key3"), 3);
	EXPECT_EQ_INT(dict_get_value(d, "key4"), 4);
	EXPECT_EQ_INT(dict_get_value(d, "key5"), 5);
	EXPECT_EQ_INT(dict_get_value(d, "key6"), 6);
	EXPECT_EQ_INT(dict_get_value(d, "key7"), 7);
	EXPECT_EQ_INT(dict_get_value(d, "key8"), 8);
	EXPECT_EQ_INT(dict_get_value(d, "key9"), 9);
	EXPECT_EQ_INT(dict_get_value(d, "key10"), 10);
	dict_print_arr(d);
	printf("--------dict_test_add-------- \n\n");
}

void dict_test_del(Dict* d) {		
	printf("--------dict_test_del-------- \n");
	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key1"), DICT_OK);
	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key1"), DICT_ERR);
	EXPECT_EQ_INT(dict_get_value(d, "key1"), NULL);
	EXPECT_EQ_INT(d->ht_.used_, 9);
	dict_print_arr(d);

	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key2"), DICT_OK);
	EXPECT_EQ_INT(dict_get_value(d, "key2"), NULL);
	EXPECT_EQ_INT(d->ht_.used_, 8);
	dict_print_arr(d);

	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key3"), DICT_OK);
	EXPECT_EQ_INT(dict_get_value(d, "key3"), NULL);
	EXPECT_EQ_INT(d->ht_.used_, 7);
	dict_print_arr(d);

	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key4"), DICT_OK);
	EXPECT_EQ_INT(dict_get_value(d, "key4"), NULL);
	EXPECT_EQ_INT(d->ht_.used_, 6);
	dict_print_arr(d);

	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key5"), DICT_OK);
	EXPECT_EQ_INT(dict_get_value(d, "key5"), NULL);
	EXPECT_EQ_INT(d->ht_.used_, 5);
	dict_print_arr(d);

	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key6"), DICT_OK);
	EXPECT_EQ_INT(dict_get_value(d, "key6"), NULL);
	EXPECT_EQ_INT(d->ht_.used_, 4);
	EXPECT_EQ_INT(d->ht_.size_, 4);
	dict_print_arr(d);

	EXPECT_EQ_INT(dict_get_value(d, "key7"), 7);
	EXPECT_EQ_INT(dict_get_value(d, "key8"), 8);
	EXPECT_EQ_INT(dict_get_value(d, "key9"), 9);
	EXPECT_EQ_INT(dict_get_value(d, "key10"), 10);
	dict_print_arr(d);
	printf("--------dict_test_del-------- \n\n");
}

void dict_test() {
	DictType type = { NULL,NULL,dictstrcmp,NULL,
					  NULL,dict_str_hashfunction };
	Dict* d = new_dict(&type);
	dict_test_add(d);
	dict_test_del(d);
}

void page_test_writefilehead(FHead* f) {
	
}

void page_test() {
	FileHeadData* fhd = new_file_head_data(4,)
}

int main(void) {

	dict_test();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
}