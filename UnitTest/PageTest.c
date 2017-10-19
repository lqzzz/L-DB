#include"TestFrameWork.h"
#include"../StorageEngine/Page.h"
static void page_head_test(FHead* f) {
	init_file(f);
	FHead* fh = read_file_head("test.data", f->filehead->row_len, 
		f->filehead->row_slot_count);
	EXPECT_EQ_INT(f->filehead->row_len, fh->filehead->row_len);
}

static void page_next_test(FHead* f) {

	Page* p = new_page(f->filehead->row_len, f->filehead->row_slot_count);
	for (size_t i = 0; i < f->filehead->page_count; i++) {
		load_page(p, i, f);
		EXPECT_EQ_INT(i, p->pdata.page_id);
		EXPECT_EQ_INT(f->filehead->row_len, p->row_len);
	}
	//mem_free(p);
}

static void page_test_add_row(FHead* f) {
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

void page_test(void) {
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

