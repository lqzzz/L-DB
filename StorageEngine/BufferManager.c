#include "../BaseStruct/Listhead.h"
#include "BufferManager.h"
#include "Page.h"
#include"../Mem/MemPool.h"
#include<string.h>
typedef struct piter{
	size_t pid;
	size_t rid;
	FHead file;
	Page page;
	PBM bm;
	const char* row;
};
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

static Page get_page(PBM bm, FHead file_head, size_t pid) {
	Page page_;
	if ((page_ = file_get_mem_page(file_head, pid)) == NULL) {
		page_ = load_page(file_head, pid);
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

void new_buffermanager(int dbid) {
	PBM bm_ = mem_alloc(sizeof(*bm_));
	bm_->DB_id = dbid;
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

int buf_insert(PBM bm, const char* filename, const char* row){
	FHead file_ = find_file_head(bm, filename);
	int page_id;
	Page page_;
	if ((page_id = file_get_insert_pid(file_)) == P_ERROR) {
		page_ = file_new_page(file_, file_->info.page_count++);
		if (bm->p_lru_list == NULL)
			bm->p_lru_list = page_;
		else LIST_ADD_TAIL(&bm->p_lru_list->head, &page_->head);

	}else 
		page_ = get_page(bm, file_, page_id);

	page_add_row(page_, page_get_empty_slot(page_), row);

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

PIterator get_table_iter(PBM bm, const char* filename){
	PIterator i = mem_alloc(sizeof(*i));
	i->bm = bm;
	i->rid = 0;
	FHead f = i->file = find_file_head(bm, filename);
	Page p = i->page = get_page(bm, f, i->pid = 0);
	i->row = p->rows_head;
	return i;
}

const char* next_row(PIterator i){
	char* row;
	if ((row = i->row) == NULL)
		return NULL;

	if (i->rid != i->page->info.used_slot_size) {
		i->row = i->page->rows_head + i->page->row_len * ++i->rid;
		return row;
	}

	if (++i->pid < i->file->info.page_count) {
		i->rid = 0;
		i->row = (i->page = get_page(i->bm, i->file, i->pid))
			->rows_head;
		return row;
	}

	i->row = NULL;

	return row;

}
