#include "../BaseStruct/Listhead.h"
#include "../Catalog.h"
#include "../BaseStruct/Vector.h"

#include "FileHead.h"
#include "BufferManager.h"
#include "Page.h"

struct PageFrame{
	int32_t hot_;
	int16_t is_dirty;
	Page page_;
};

struct bm {
	Listhead bm_list;
	int DB_id;
	Vector file_head_list;//<FHead>
	Vector used_page_list;//<PageFram>
	Vector empty_page_list;//<PageFram>
	//Vector other_page_list;
};

typedef struct bm* Ptr;
typedef struct PageFrame* FPtr;

static Ptr bm_list_head = NULL;
static const size_t free_page_count = 4;

static FPtr new_pageframe(char* tablename);

Ptr get_bm_head(void) {
	return bm_list_head;
}

__inline static FPtr get_frame(Page* page) {
	return (FPtr)((char*)page - sizeof(int32_t) - sizeof(int16_t));

}

FPtr new_pageframe(char* tablename) {
	if()
	return mem_calloc(1, sizeof(struct PageFrame) + PageSize);
}

__inline static FHead* find_file_head(Ptr bm,char* filename) {
	FHead* file_head = NULL;
	VECTOR_FOREACH(file_head, &bm->file_head_list,
		if (strcmp(file_head->filename_, filename) == 0)
			break;
		);
	return file_head;
}

void add_bm(DBnode* db) {

}

void new_bufferManager(DBnode* db) {
	Ptr bm_ = mem_alloc(sizeof(*bm_));
	bm_->DB_id = db->id_;
	LIST_INIT(&bm_->bm_list);

	VECTOR_INIT(&bm_->file_head_list, INIT_LEN);
	VECTOR_INIT(&bm_->empty_page_list, free_page_count);
	VECTOR_INIT(&bm_->used_page_list, INIT_LEN);

	Vector* v_ = &bm_->empty_page_list;
	for (size_t i = 0; i < free_page_count; i++) 
		VECTOR_PUSHBACK(v_, new_pageframe());

	if (bm_list_head == NULL)
		bm_list_head = bm_;
	else LIST_ADD_TAIL(&bm_list_head->bm_list, &bm_->bm_list);
}

void bm_add_file_head(int DBid, FHead* filehead) {
	VECTOR_PUSHBACK(&(get_buffman(DBid)->file_head_list), filehead);
}

__inline Page* alloc_page(Ptr bm) {
	FPtr fp = vector_pop(&bm->empty_page_list);
	if (fp) 
		VECTOR_PUSHBACK(&bm->used_page_list, fp);
	else 
		VECTOR_PUSHBACK(&bm->used_page_list, fp = new_pageframe());
	return &fp->page_;
}

//char* get_next_row(Ptr bm,char* filename, size_t *pageiter, size_t *rowiter) {
//	FHead* file_head = NULL;
//	Page* page_ = NULL;
//	FPtr pf_ = NULL;
//	char* next_row = NULL;
//	size_t page_id = 0;
//	file_head = find_file_head(bm, filename);
//
//	if (page_id = next_page(file_head, pageiter) == -1)
//		return NULL;
//
//	if ((page_ = get_page_ptr(file_head, page_id)) == NULL) {
//		page_ = alloc_page(bm);
//		load_page(page_, page_id, file_head);
//	}
//
//	next_row = page_next_row(page_, rowiter);
//
//	//next page
//	if (next_row == NULL) {
//		//todo sort hot_ 
//		(*pageiter)++;
//		return get_next_row(bm, filename, pageiter, rowiter);
//	}
//	pf_ = get_frame(page_);
//	pf_->hot_++;
//	return next_row;
//}

// !sorted
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