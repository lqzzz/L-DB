#include"TestFrameWork.h"
#include"../StorageEngine/Page.h"
#include"../StorageEngine/BufferManager.h"
static FHead file;
static Page p1, p2;
static void test_init() {
	size_t rowlen = 36;
	size_t rps = PageSize - 2 * sizeof(size_t);
	size_t rsn = rps / rowlen;
	while (rsn * rowlen + rsn > rps) rsn--;
	file = new_file_head("test.data", rsn, 36);
}

static void file_init_test() {
	init_file(file);
	FHead fh = read_file_head("test.data");
	EXPECT_EQ_INT(file->info.row_len, fh->info.row_len);
}

static void file_get_insert_test() {
	EXPECT_EQ_INT(P_ERROR, file_get_insert_pid(file));
	Page p = file_new_page(file,0);
	EXPECT_EQ_INT(0, file_get_insert_pid(file));
	page_free(p);
	file->info.page_count--;
}

static file_get_mem_test() {
	Page p = file_get_mem_page(file, 0);
	EXPECT_EQ_INT(NULL, p);
	file_new_page(file,0);
	p = file_get_mem_page(file, 0);
	EXPECT_EQ_INT(file->info.row_slot_count, p->slot_count);
	EXPECT_EQ_INT(0, p->page_id);
	page_free(p);
	file->info.page_count--;
	EXPECT_EQ_INT(0, file->info.page_count);
	p = file_get_mem_page(file, 0);
	EXPECT_EQ_INT(NULL, p);
}

static void file_head_test() {
	file_init_test();
	file_get_insert_test();
	file_get_mem_test();
}

static void page_test(FHead f) {


}

static void page_test_add_row(FHead f) {
	Page ps = file_new_page(f, 0);
	Page pd = file_new_page(f, 3);

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

	ps = file_get_mem_page(f, 0);
	pd = file_get_mem_page(f, 3);
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
	EXPECT_EQ_INT(NULL,file_get_mem_page(f, 0));

	ps = file_get_new_page(f, 0);
	pd = file_get_new_page(f, 3);
	EXPECT_EQ_STR(page_get_row(ps,0), "ps r1");
	EXPECT_EQ_STR(page_get_row(ps,25), "ps r26");
	EXPECT_EQ_STR(page_get_row(ps,26), "ps r27");
	EXPECT_EQ_STR(page_get_row(pd,0), "pd r1");
	EXPECT_EQ_STR(page_get_row(pd,25), "pd r26");
	EXPECT_EQ_STR(page_get_row(pd,26), "pd r27");
}

void file_page_test(void) {
	test_init();
	file_head_test();

	remove(file->filename_);
}

