#include "../BaseStruct/Listhead.h"
#include "../Catalog.h"
#include "../BaseStruct/Vector.h"
#include "BufferManager.h"
#include "Page.h"

#define DIRTY 1
#define NOT_DIRTY 0

struct PageFrame{
	Listhead head;
	int16_t is_dirty;
	int32_t hot_;
	Page page_;
};

typedef struct PageFrame* FPtr;

struct bm {
	Listhead bm_list;
	int DB_id;
	FHead* file_head_list;
	FPtr lru_page_list;
	FHead* temp_head_list;
};

typedef struct bm* Ptr;

static Ptr bm_list_head = NULL;

static FPtr new_pageframe(const char* tablename);
__inline static FHead* find_file_head(Ptr bm, const char* filename);

Ptr get_bm_head(void) {
	return bm_list_head;
}

static FPtr get_frame(Page* page) {
	return (FPtr)((char*)page - (sizeof(struct PageFrame) - sizeof(Page)));
}

FPtr new_pageframe(size_t rowlen,size_t rowslotcount) {
	//if (!file_head)
	//	return NULL;
	FPtr fp = mem_alloc(sizeof(*fp) - sizeof(PageData) + PageSize);
	LIST_INIT(&fp->head);
	fp->hot_ = 1;
	fp->is_dirty = DIRTY;
	page_init(&fp->page_, rowlen, rowslotcount);
	return fp;
}

__inline static FHead* find_file_head(Ptr bm,const char* filename) {
	FHead* file_head = NULL;
	char flag = 0;
	LIST_FOREACH(file_head, &bm->file_head_list,
		if (strcmp(file_head->filename_, filename) == 0) {
			flag = 1;
			break;
		}
	);
	return flag ? file_head : NULL;
}

void add_bm(DBnode* db) {

}

void new_bufferManager(DBnode* db) {
	Ptr bm_ = mem_alloc(sizeof(*bm_));
	bm_->DB_id = db->id_;
	LIST_INIT(&bm_->bm_list);
	if (bm_list_head == NULL)
		bm_list_head = bm_;
	else LIST_ADD_TAIL(&bm_list_head->bm_list, &bm_->bm_list);
}

void bm_add_raw_file_head(Ptr bm, FHead* filehead) {
	LIST_ADD_TAIL(&bm->file_head_list->head, &filehead->head);
}

char* buf_get_row(BufferManager* bm, const char* filename, int* pid, int* rid){
	FHead* fh;
	return fh = find_file_head(bm, filename) ?
		file_get_row(fh, *pid, *rid) : NULL;
}

char* scan_table(char* tablename, int* pid, int* rid){
	
	return NULL;
}

char* get_next_row(Ptr bm, char* filename, size_t *pageiter, size_t *rowiter) {
	FHead* file_head;
	Page* page_;
	FPtr pf_;
	char* next_row;
	size_t page_id;
	if (*pageiter > PageCount)
		return NULL;
	file_head = find_file_head(bm, filename);

	if ((page_id = next_page_id(file_head, pageiter)) == P_ERROR)
		return NULL;

	if ((page_ = file_get_page(file_head, page_id)) == NULL) {
		pf_ = new_pageframe(file_head->filehead->row_len,
			file_head->filehead->row_slot_count);
		load_page(page_ = &pf_->page_, page_id, file_head);
	}

	next_row = page_next_row(page_, rowiter);

	//next page
	if (next_row == NULL) {
		//todo sort hot_ 
		(*pageiter)++;
		return get_next_row(bm, filename, pageiter, rowiter);
	}
	pf_ = get_frame(page_);
	pf_->hot_++;
	return next_row;
}

//!sorted

//void page_fill(Ptr bm, char* filename, VectorIter* rowiter) {
//	FHead* file_ = find_file_head(bm, filename);
//	Page* page_ = NULL;
//	while (vector_has_next(rowiter)) {
//		int page_id = get_not_full_page_id(file_);
//		//todo page_id = -1;
//		if ((page_ = get_page_ptr(file_, page_id)) == NULL) {
//			page_ = alloc_page(bm);
//			load_page(page_, page_id, file_);
//		}
//		if (page_add_row(page_, rowiter) == 1) 
//			set_page_full_state(file_, page_id, 1);
//		FPtr frame_ = get_frame(page_);
//		frame_->is_dirty = 1;
//		frame_->hot_++;
//	}
//	//store_page(page_, file_);
//}

Ptr get_buffman(int DBid){
	Ptr bm_ = NULL;
	LIST_FOREACH(bm_, bm_list_head,
		if (bm_->DB_id == DBid)
			break;
	);
	return bm_;
}