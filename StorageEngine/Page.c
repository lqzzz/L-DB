#include<stdio.h>
#include"../Mem/MemPool.h"
#include"Page.h"

static DictType dt = { NULL,NULL,NULL,NULL,NULL,dict_int_hashfunction };
__inline int page_has_next(FHead* p, size_t index) {
	return index == p->filehead->page_count ? P_ERROR : P_OK;
}

__inline int row_has_next(Page* p,size_t rowindex) {
	return rowindex == p->slot_count ? P_ERROR : P_OK;
}

//必须调用过init_file
void write_file_head(const FHead* p){
	FILE* fd_ = fopen(p->filename_, "rb+");
	fwrite(p->filehead, PageSize, 1, fd_);
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

char * file_get_row(FHead* fh, size_t pageid, size_t rowindex){
	return page_get_row(file_get_page_by_id(fh, pageid), rowindex);
}

int file_add_row(FHead* fh, size_t pageid, size_t rowindex,const char* row){
	if(fh->page_states[pageid] == P_FULL)
		return P_ERROR;

	Page* p = file_get_page_by_id(fh, pageid);

	if (page_add_row(p,rowindex,row) == P_ERROR)
		return P_ERROR;
	if (p->pdata.used_slot_size == p->slot_count)
		fh->page_states[pageid] = P_FULL;
	return P_OK;
}

FHead* new_file_head(char* filename, FileHeadData* fhd){
	FHead* fh = mem_alloc(sizeof(*fh));
	//
	fh->filename_ = filename;
	fh->page_id_map = new_dict(&dt);
	fh->filehead = fhd;
	fh->page_states = fhd->page_state_head;
	return fh;
}

FHead* read_file_head(const char* filename,size_t rowlen,size_t rowslotcount) {
	FILE* fd_;
	fd_ = fopen(filename, "rb");
	//if (fd_ == NULL) return 1;
	FileHeadData* fhd = mem_alloc(PageSize);

	fseek(fd_, 0, SEEK_CUR);

	fread(fhd, PageSize, 1, fd_);

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
	//dict_del_entry_nofree(p->page_id_map, 
	//	page->pdata.page_id);
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
		return next_page_id(p, index);
	}
	return *index;
}

Page* file_get_page_by_id(FHead* f ,size_t pageid) {
	Page* p;
	if ((p = dict_get_value(f->page_id_map, pageid)))
		return p;
	p = new_page(f->filehead->row_len, 
		f->filehead->row_slot_count);
	load_page(p, pageid, f);
	return p;
}

void set_page_full_state(FHead* p, int id,char state){
	p->filehead->page_state_head[id] = state;
}

void page_init(Page* p,size_t rowlen, size_t slot_count){
	p->row_len = rowlen;
	p->slot_count = slot_count;
	p->slot_state_head_ptr = p->pdata.row_set;
	p->rows_head = p->pdata.row_set + slot_count * sizeof(char);
	//p->slot_state_head_ptr = &p->pdata.s;
	//p->rows_head = p->pdata.row_set;
}

Page* new_page(size_t rowlen, size_t slot_count){
	Page* p = mem_alloc(sizeof(Page) - 
		sizeof(PageData) + PageSize);
	page_init(p, rowlen, slot_count);
	return p;
}

void pagedata_init(PageData* pd){

}

void page_del(FHead* f, Page* p){
	dict_del_entry_nofree(f->page_id_map, p->pdata.page_id);
}

char * page_get_row(Page * p, size_t index){
	return p->slot_state_head_ptr[index] == P_EMPTY ? NULL :
		p->rows_head + index * p->row_len;
}

int page_add_row(Page* p, size_t slot_index,const char* row) {
	if (slot_index >= p->slot_count) 
		return P_ERROR;

	if (p->slot_state_head_ptr[slot_index] == P_NOT_EMPTY)
		return P_ERROR;

	size_t offset = slot_index* p->row_len;

	char* prow = p->rows_head + offset;

	memcpy(prow, row, p->row_len);

	//puts(prow);

	p->slot_state_head_ptr[slot_index] = P_NOT_EMPTY;
	p->pdata.used_slot_size++;

	//for (size_t i = 0; i < p->slot_count; i++) {
	//	char s = p->slot_state_head_ptr[i];
	//	if (s == P_EMPTY)
	//		printf("%d", 0);
	//	else printf("%c", s);
	//}
	//printf("\n");

	return P_OK;
}



char* page_next_row(Page* p,size_t* rowiter) {
	if (row_has_next(p, *rowiter) == P_ERROR)
		return NULL;
	if (p->slot_state_head_ptr[*rowiter] == P_EMPTY) {
		++(*rowiter);
		return page_next_row(p, rowiter);
	}
	size_t offset = (*rowiter)++ * p->row_len;
	return p->rows_head + offset;
}

