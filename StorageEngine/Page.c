#include<stdio.h>
#include"../Mem/MemPool.h"
#include"Page.h"

__inline int page_has_next(FHead* p, size_t index) {
	return index == p->filehead->page_count ? P_ERROR : P_OK;
}

__inline int row_has_next(Page* p,size_t rowindex) {
	return rowindex == p->slot_count ? P_ERROR : P_OK;
}

void write_file_head(FHead* p){
	FILE* fd_ = fopen(p->filename_, "rb+");
	fwrite(&p->filehead, PageSize, 1, fd_);
	fflush(fd_);
	fclose(fd_);
}

FileHeadData* new_file_head_data(size_t pagecount, size_t rowslotcount, size_t rowlen){
	FileHeadData* fhg = mem_alloc(PageSize);
	fhg->page_count = pagecount;
	fhg->row_len = rowlen;
	fhg->row_slot_count = rowslotcount;
	fhg->used_size = 0;
	memset(fhg->page_state_head, 0, pagecount);
	return fhg;
}

void init_file(FHead *fh){

	fclose(fopen(fh->filename_, "a"));
	//
	write_file_head(fh);

	struct {
		size_t page_id;
		size_t used_slot_size;
		char slot_state_head[];
	}*initdata;

	size_t pagecount = fh->filehead->page_count;

	size_t len = sizeof(*initdata) +
		fh->filehead->row_slot_count * sizeof(char);
	
	initdata = mem_calloc(1, len);

	size_t offset = PageSize - len;

	FILE* f = fopen(fh->filename_, "rb+");

	fseek(f, PageSize, SEEK_CUR);

	for (size_t i = 0; i < pagecount; i++) {
		fwrite(initdata, len, 1, f);
		initdata->page_id++;
		fseek(f, offset, SEEK_CUR);
	}

	mem_free(initdata);
	fflush(f);
	fclose(f);
	
}

FHead* new_file_head(char* filename, FileHeadData* fhd){
	FHead* fh = mem_alloc(sizeof(*fh));
	//
	fh->filename_ = filename;
	fh->page_id_map = new_dict(NULL);
	fh->filehead = fhd;
	return fh;
}

FHead* read_file_head(char* filename,size_t rowlen,size_t rowslotcount) {
	FILE* fd_;
	fd_ = fopen(filename, "rb");
	//if (fd_ == NULL) return 1;
	FileHeadData* fhd = mem_alloc(PageSize);

	fseek(fd_, 0, SEEK_CUR);

	fread(&fhd, PageSize, 1, fd_);

	fclose(fd_);
	return new_file_head(filename, fhd);
}

int load_page(Page* page,int id,FHead* p){
	FILE* fd_;

	fd_ = fopen(p->filename_, "rb");
	//if (fd_ == NULL) return 1;

	fseek(fd_, (id + 1) * PageSize, SEEK_CUR);

	page->slot_count = p->filehead->row_slot_count;
	page->row_len = p->filehead->row_len;

	fread(&page->pdata, PageSize, 1, fd_);
	fclose(fd_);
	dict_add_entry(p->page_id_map, page->pdata.page_id, page);
	return 1;
}

int store_page(Page* page,FHead* p){
	FILE* fd_;
	fd_ = fopen(p->filename_, "rb+");
	//
	fseek(fd_, (page->pdata.page_id + 1) * PageSize, SEEK_CUR);

	fwrite(&page->pdata, PageSize, 1, fd_);

	fclose(fd_);
	dict_del_entry(p->page_id_map, page->pdata.page_id);
	return 1;
}

int file_get_not_full_page_id(FHead* f) {
	char* page_state_head = f->filehead->page_state_head;
	size_t page_count = f->filehead->page_count;
	int page_id = -1;
	
	for (size_t i = 0; i < page_count; i++) 
		if (page_state_head[i] != P_FULL) {
			page_id = i;
			break;
		}
	//todo can't find 
	return page_id;
}

char * get_file_name(FHead* p){
	return p->filename_;
}

int next_page_id(FHead* p, size_t* index) {
	if (page_has_next(p, *index) == P_ERROR);
		return P_ERROR;
	if (p->filehead->page_state_head[*index] == P_EMPTY) {
		++(*index);
		return next_page(p, index);
	}
	return *index;
}

Page* file_get_page_by_id(FHead* p ,size_t pageid) {
	return dict_get_value(p->page_id_map, pageid);
}

void set_page_full_state(FHead* p, int id,char state){
	p->filehead->page_state_head[id] = state;
}

void page_init(Page* p,size_t rowlen, size_t slot_count){
	p->row_len = rowlen;
	p->slot_count = slot_count;
	p->slot_state_head_ptr = &p->pdata.slot_state_head;
}

Page* new_page(size_t rowlen, size_t slot_count){
	Page* p = mem_alloc(sizeof(Page) - 
		sizeof(PageData) + PageSize);
	page_init(p, rowlen, slot_count);
	return p;
}

void pagedata_init(PageData* pd){

}

int page_add_row(Page* p, size_t slot_index,char* row) {
	if (slot_index >= p->slot_count) 
		return P_ERROR;

	char* state_iter = &p->slot_state_head_ptr;
	state_iter += slot_index;
	if (*state_iter) 
		return P_ERROR;

	size_t offset = slot_index* p->row_len;

	char* prow = p->pdata.row_set + offset;

	memcpy(prow, row, p->row_len);
	*state_iter = P_NOT_EMPTY;
	return P_OK;
}



char* page_next_row(Page* p,size_t* rowiter) {
	if (row_has_next(p, *rowiter) == P_ERROR)
		return P_ERROR;

	if (p->slot_state_head_ptr[*rowiter] == P_EMPTY) {
		++(*rowiter);
		return page_next_row(p, rowiter);
	}
	size_t offset = (*rowiter)++ * p->row_len;
	return p->pdata.row_set + offset;

}

