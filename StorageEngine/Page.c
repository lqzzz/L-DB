#include<stdio.h>
#include"../Mem/MemPool.h"
#include"Page.h"

int get_row_index(Page p,size_t rowindex) {
	if (rowindex >= p->info.used_slot_size)
		return P_ERROR;
	return rowindex;
}

//必须调用过init_file
void write_file_head(CFHead f){
	FILE* fd_ = fopen(f->filename_, "rb+");
	fwrite(&f->info, PageSize, 1, fd_);
	fflush(fd_);
	fclose(fd_);
}

void init_file(FHead fh){
	fclose(fopen(fh->filename_, "a"));

	FILE* f = fopen(fh->filename_, "rb+");

	fwrite(&fh->info, PageSize, 1, f);

	fflush(f);
	fclose(f);
}

	//struct {
	//	size_t page_id;
	//	size_t used_slot_size;
	//	char slot_state_head[];
	//}*initdata;

	//size_t pagecount = fh->info.page_count;

	//size_t len = sizeof(*initdata) +
	//	fh->info.row_slot_count * sizeof(char);
	//
	//initdata = mem_calloc(1, len);

	//size_t offset = PageSize - len;

	//for (size_t i = 0; i < fh->info.page_count; i++) {
	//	fwrite(initdata, len, 1, f);
	//	initdata->page_id++;
	//	fseek(f, offset, SEEK_CUR);
	//}

FHead new_empty_file() {
	FHead fh = mem_alloc(sizeof(struct filehead) + PageSize);
	LIST_INIT(&fh->head);
	return fh;
}

FHead* new_file_head(const char* filename,size_t pagecount,
	size_t rowslotcount,size_t rowlen){

	FHead fh = new_empty_file();
	fh->filename_ = filename;
	fh->info.page_count = pagecount;
	fh->info.row_len = rowlen;
	fh->info.row_slot_count = rowslotcount;
	fh->mem_page_bit_map = mem_calloc(1, sizeof(Page) * pagecount);
	//memset(fh->mem_page_bit_map, 0, pagecount*size_t);
	return fh;
}

FHead read_file_head(const char* filename,size_t rowlen,size_t rowslotcount) {
	FILE* fd_;

	FHead f = new_empty_file();

	fd_ = fopen(filename, "rb");

	fseek(fd_, 0, SEEK_CUR);

	fread(&f->info, PageSize, 1, fd_);

	fclose(fd_);

	f->filename_ = filename;
	f->mem_page_bit_map = mem_calloc(1, sizeof(Page) * f->info.page_count);

	return f;
}

int load_page(FHead f,size_t id,Page page){
	FILE* fd_;

	fd_ = fopen(f->filename_, "rb");

	fseek(fd_, (id + 1) * PageSize, SEEK_CUR);

	fread(&page->info, PageSize, 1, fd_);

	fclose(fd_);

	page->slot_count = f->info.row_slot_count;
	page->row_len = f->info.row_len;
	f->mem_page_bit_map[id] = page;

	return P_OK;
}

int store_page(Page page,FHead f){
	FILE* fd_;
	fd_ = fopen(f->filename_, "rb+");

	fseek(fd_, (page->info.page_id + 1) * PageSize, SEEK_CUR);

	fwrite(&page->info, PageSize, 1, fd_);

	fclose(fd_);
	f->mem_page_bit_map[page->info.page_id] = NULL;

	return 1;
}

int page_free(Page p){
	//mvccrows_free(p->rows_head)
	mem_free(p);
	return P_OK;
}

int file_get_not_full_page_id(FHead f) {

	for (size_t i = 0; i < f->info.page_count; i++) 
		if (f->info.page_states[i] != P_FULL) 
			return i;

	Page p = new_page(f->info.row_len, f->info.row_slot_count);

	return p->info.page_id = f->info.page_count++;
}

char * get_file_name(FHead p){
	return p->filename_;
}

int next_page_id(FHead f, size_t* index) {
	if (page_has_next(f, *index) == P_ERROR);
		return P_ERROR;
	if (f->info.page_states[*index] == P_EMPTY) {
		++(*index);
		return next_page_id(f, index);
	}
	return *index;
}

int mvccrows_free(MvccRows mr){

	return 0;
}

Page file_get_mem_page(FHead f ,size_t pageid) {
	if (pageid >= f->info.page_count)
		return NULL;
	Page *p;
	if (p = f->mem_page_bit_map[pageid])
		return p;
}

Page file_get_new_page(FHead f ,size_t pageid) {
	Page *p = new_page(f->info.row_len, f->info.row_slot_count);
	load_page(f, pageid, p);
	f->mem_page_bit_map[pageid] = p;
	return p;
}

void set_page_full_state(FHead f, int id,char state){
	f->info.page_states[id] = state;
}

Page new_empty_page(void) {
	Page p = mem_calloc(sizeof(struct phead) + PageSize);
	LIST_INIT(&p->head);
	//p->is_dirty = 0;
	//p->info.used_slot_size = 0;
	return p;
}

Page new_page(size_t rowlen, size_t slot_count){
	Page p = new_empty_page();
	p->row_len = rowlen;
	p->slot_count = slot_count;
	p->rows_head = p->info.slot_states + slot_count * sizeof(char);
	p->arr_mrows = mem_alloc(sizeof(MvccRows) * slot_count);
	return p;
}

void page_del(FHead f, Page p){
	f->mem_page_bit_map[p->info.page_id] = NULL;
	mem_free(p);
}

int page_add_row(Page p, int index,const char* row){
	
	size_t offset = index* p->row_len;

	char* prow = p->rows_head + offset;

	memcpy(prow, row, p->row_len);
	p->info.used_slot_size++;
	p->info.slot_states[index] = R_NOT_EMPTY;
	return P_OK;
}

int page_get_empty_slot(Page p){
	for (int index = 0; p->slot_count; index++) {
		if (p->info.slot_states[index] == R_EMPTY)
			return index;
	}
	return P_ERROR;
}

char* page_get_row(Page p, size_t index){
	return p->rows_head + index * p->row_len;
}

//int file_add_row(FHead f,const char* row) {
//	int pid;
//	if (pid = file_get_not_full_page_id(f) == -1)
//		return P_ERROR;
//
//	if (p->slot_state_head_ptr[index] == P_NOT_EMPTY)
//		return file_add_row(f,p, slot_index++, row);
//
//	size_t offset = slot_index* p->row_len;
//
//	char* prow = p->rows_head + offset;
//
//	memcpy(prow, row, p->row_len);
//	p->slot_state_head_ptr[slot_index] = P_NOT_EMPTY;
//	
//	if (++p->info.used_slot_size == p->slot_count)
//		f->page_states[p->info.page_id] = P_FULL;
//		
//	return P_OK;
//}

char* page_next_row(const Page p,size_t* rowiter) {
	if (row_has_next(p, *rowiter) == P_ERROR)
		return NULL;

	if (p->info.slot_states[*rowiter] == R_EMPTY) {
		++(*rowiter);
		return page_next_row(p, rowiter);
	}
	return p->rows_head + (*rowiter)++ * p->row_len;
}

