#include "../BaseStruct/Listhead.h"
#include "BufferManager.h"
#include "Page.h"

#define DIRTY 1
#define NOT_DIRTY 0

struct buffermanager {
	Listhead bm_list;
	int DB_id;
	FHead file_head_list;
	FHead temp_head_list;
	Page p_lru_list;
};

typedef struct buffermanager* PBM;

static PBM bm_list_head = NULL;

__inline static FHead find_file_head(PBM bm, const char* filename);
__inline static FHead find_file_head(PBM bm, const char* filename);

static Page get_page(PBM bm, const char* filename, size_t pid) {
	if (pid > PageCount)
		return NULL;
	FHead file_head = find_file_head(bm, filename);
	size_t page_id;
	if ((page_id = next_page_id(file_head, &pid)) == P_ERROR)
		return NULL;
	Page page_;
	if ((page_ = file_get_mem_page(file_head, page_id)) == NULL) {
		page_ = file_get_new_page(file_head, page_id);
		if (bm->p_lru_list == NULL)
			bm->p_lru_list = page_;
		else LIST_ADD_TAIL(&bm->p_lru_list->head, &page_->head);
	}
	return page_;
}

PBM get_bm_head(void) {
	return bm_list_head;
}

static FHead find_file_head(PBM bm,const char* filename) {
	FHead file_head = NULL;
	char flag = 0;
	int s = strcmp(bm->file_head_list->filename_, filename);
	LIST_FOREACH(file_head, &bm->file_head_list->head,
		if (strcmp(file_head->filename_, filename) == 0) {
			flag = 1;
			break;
		}
	);
	return flag ? file_head : NULL;
}

void new_buffermanager(DBnode* db) {
	PBM bm_ = mem_alloc(sizeof(*bm_));
	bm_->DB_id = db->id_;
	LIST_INIT(&bm_->bm_list);
	bm_->file_head_list = NULL;

	if (bm_list_head == NULL)
		bm_list_head = bm_;
	else LIST_ADD_TAIL(&bm_list_head->bm_list, &bm_->bm_list);

}

void bm_add_raw_file_head(PBM bm, FHead filehead) {
	if (bm->file_head_list == NULL)
		bm->file_head_list = filehead;
	else LIST_ADD_TAIL(&bm->file_head_list->head, &filehead->head);
}

Page buf_get_page(PBM bm, const char* filename, int pid) {
	if (pid > PageCount)
		return NULL;

	FHead file_head = find_file_head(bm, filename);
	Page page_;
	
	if ((page_ = file_get_mem_page(file_head, pid)) == NULL) {
		//
		page_ = file_get_new_page(file_head, pid);
		if (bm->p_lru_list == NULL)
			bm->p_lru_list = page_;
		else LIST_ADD_TAIL(&bm->p_lru_list->head, &page_->head);
	}
	return page_;

}

int buf_insert(PBM* bm, const char* filename, const char* row){
	FHead file_ = find_file_head(bm, filename);

	int page_id = file_get_not_full_page_id(file_);
	
	Page page_ = get_page(bm,filename,page_id);

	int slot_index = page_get_empty_slot(page_);

	page_add_row(page_, slot_index, row);

	if (page_->slot_count == page_->info.used_slot_size)
		file_->info.page_states[page_id] = P_FULL;
	page_->is_dirty = 1;

	return P_OK;
}

PBM get_buffman(int DBid){
	PBM bm_ = NULL;
	LIST_FOREACH(bm_, bm_list_head,
		if (bm_->DB_id == DBid)
			break;
	);
	return bm_;
}