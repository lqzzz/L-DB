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
#define EXPECT_EQ_STR(expect, actual) EXPECT_EQ_BASE(dictstrcmp((expect),(actual)), expect, actual, "%s")

int dictstrcmp(const char* str1, const char* str2) {
	return strcmp(str1, str2) == 0 ? 1 : 0;
}
//dict_print_arr(Dict* d) {
//	printf("hasharray : ");
//	for (size_t i = 0; i < d->ht_.size_; i++){
//		int num = 0;
//		for (DictEntry* e = d->ht_.table_[i]; e; num++, e = e->next_);
//		printf("%d ", num);
//	}
//	printf("\n");
//}
//
//void dict_test_add(Dict* d) {
//	printf("--------dict_test_add-------- \n");
//	dict_add_entry(d, "key1", 1);
//	EXPECT_EQ_INT(DICT_HT_INITIAL_SIZE, d->ht_.size_);
//	EXPECT_EQ_INT(1, d->ht_.used_);
//	dict_add_entry(d, "key2", 2);
//	dict_add_entry(d, "key3", 3);
//	dict_add_entry(d, "key4", 4);
//	dict_print_arr(d);
//	dict_add_entry(d, "key5", 5);
//	dict_add_entry(d, "key6", 6);
//	dict_add_entry(d, "key7", 7);
//	dict_print_arr(d);
//	dict_add_entry(d, "key8", 8);
//	dict_add_entry(d, "key9", 9);
//	dict_add_entry(d, "key10", 10);
//	dict_print_arr(d);
//	EXPECT_EQ_INT(10, d->ht_.used_);
//	EXPECT_EQ_INT(8, d->ht_.size_);
//	EXPECT_EQ_INT(dict_get_value(d, "key1"), 1);
//	EXPECT_EQ_INT(dict_get_value(d, "key2"), 2);
//	EXPECT_EQ_INT(dict_get_value(d, "key3"), 3);
//	EXPECT_EQ_INT(dict_get_value(d, "key4"), 4);
//	EXPECT_EQ_INT(dict_get_value(d, "key5"), 5);
//	EXPECT_EQ_INT(dict_get_value(d, "key6"), 6);
//	EXPECT_EQ_INT(dict_get_value(d, "key7"), 7);
//	EXPECT_EQ_INT(dict_get_value(d, "key8"), 8);
//	EXPECT_EQ_INT(dict_get_value(d, "key9"), 9);
//	EXPECT_EQ_INT(dict_get_value(d, "key10"), 10);
//	dict_print_arr(d);
//	printf("--------dict_test_add-------- \n\n");
//}
//
//void dict_test_del(Dict* d) {		
//	printf("--------dict_test_del-------- \n");
//	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key1"), DICT_OK);
//	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key1"), DICT_ERR);
//	EXPECT_EQ_INT(dict_get_value(d, "key1"), NULL);
//	EXPECT_EQ_INT(d->ht_.used_, 9);
//	dict_print_arr(d);
//
//	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key2"), DICT_OK);
//	EXPECT_EQ_INT(dict_get_value(d, "key2"), NULL);
//	EXPECT_EQ_INT(d->ht_.used_, 8);
//	dict_print_arr(d);
//
//	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key3"), DICT_OK);
//	EXPECT_EQ_INT(dict_get_value(d, "key3"), NULL);
//	EXPECT_EQ_INT(d->ht_.used_, 7);
//	dict_print_arr(d);
//
//	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key4"), DICT_OK);
//	EXPECT_EQ_INT(dict_get_value(d, "key4"), NULL);
//	EXPECT_EQ_INT(d->ht_.used_, 6);
//	dict_print_arr(d);
//
//	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key5"), DICT_OK);
//	EXPECT_EQ_INT(dict_get_value(d, "key5"), NULL);
//	EXPECT_EQ_INT(d->ht_.used_, 5);
//	dict_print_arr(d);
//
//	EXPECT_EQ_INT(dict_del_entry_nofree(d, "key6"), DICT_OK);
//	EXPECT_EQ_INT(dict_get_value(d, "key6"), NULL);
//	EXPECT_EQ_INT(d->ht_.used_, 4);
//	EXPECT_EQ_INT(d->ht_.size_, 4);
//	dict_print_arr(d);
//
//	EXPECT_EQ_INT(dict_get_value(d, "key7"), 7);
//	EXPECT_EQ_INT(dict_get_value(d, "key8"), 8);
//	EXPECT_EQ_INT(dict_get_value(d, "key9"), 9);
//	EXPECT_EQ_INT(dict_get_value(d, "key10"), 10);
//	dict_print_arr(d);
//	printf("--------dict_test_del-------- \n\n");
//}
//
//void dict_test() {
//	DictType type = { NULL,NULL,dictstrcmp,NULL,
//					  NULL,dict_str_hashfunction };
//	Dict* d = new_dict(&type);
//	dict_test_add(d);
//	dict_test_del(d);
//}

void page_head_test(FHead* f) {
	init_file(f);
	FHead* fh = read_file_head("test.data", f->filehead->row_len, 
		f->filehead->row_slot_count);
	EXPECT_EQ_INT(f->filehead->row_len, fh->filehead->row_len);
}

void page_next_test(FHead* f) {
	//size_t *page_iter = 0;
	Page* p = new_page(f->filehead->row_len, f->filehead->row_slot_count);
	for (size_t i = 0; i < f->filehead->page_count; i++) {
		load_page(p, i, f);
		EXPECT_EQ_INT(i, p->pdata.page_id);
		EXPECT_EQ_INT(f->filehead->row_len, p->row_len);
	}
	mem_free(p);
}

void page_test_add_row(FHead* f) {
	Page* ps = file_get_page_by_id(f, 0);
	Page* pd = file_get_page_by_id(f, 3);
	page_add_row(ps, 0, "ps r1");
	page_add_row(ps, 25, "ps r26");
	page_add_row(ps, 26, "ps r27");
	page_add_row(pd, 0, "pd r1");
	page_add_row(pd, 25, "pd r26");
	page_add_row(pd, 26, "pd r27");
	EXPECT_EQ_STR(page_get_row(ps,0), "ps r1");
	EXPECT_EQ_STR(page_get_row(ps,25), "ps r26");
	EXPECT_EQ_STR(page_get_row(ps,26), "ps r27");
	EXPECT_EQ_STR(page_get_row(pd,0), "pd r1");
	EXPECT_EQ_STR(page_get_row(pd,25), "pd r26");
	EXPECT_EQ_STR(page_get_row(pd,26), "pd r27");

	store_page(ps, f);
	store_page(pd, f);
	page_del(f, ps);
	page_del(f, pd);

	ps = file_get_page_by_id(f, 0);
	pd = file_get_page_by_id(f, 3);
	EXPECT_EQ_STR(page_get_row(ps,0), "ps r1");
	EXPECT_EQ_STR(page_get_row(ps,25), "ps r26");
	EXPECT_EQ_STR(page_get_row(ps,26), "ps r27");
	EXPECT_EQ_STR(page_get_row(pd,0), "pd r1");
	EXPECT_EQ_STR(page_get_row(pd,25), "pd r26");
	EXPECT_EQ_STR(page_get_row(pd,26), "pd r27");
}

void page_test() {
	size_t rowlen = 36;
	size_t rps = PageSize - 2 * sizeof(size_t);
	size_t rsn = rps / rowlen;
	while (rsn * rowlen + rsn > rps) rsn--;
	FileHeadData* fhd = new_file_head_data(4, rsn, rowlen);
	FHead* fh = new_file_head("test.data", fhd);

	page_head_test(fh);
	//page_next_test(fh);
	page_test_add_row(fh);

	remove(fh->filename_);
}

int main(void) {

	page_test();
	//dict_test();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
}