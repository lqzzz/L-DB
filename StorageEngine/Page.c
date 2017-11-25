#include<stdio.h>
#include"../Mem/MemPool.h"
#include"Page.h"

//__inline int page_has_next(FHead* p, size_t index) {
//	return index == p->filehead->page_count ? P_ERROR : P_OK;
//}
//
//__inline int row_has_next(Page* p,size_t rowindex) {
//	return rowindex == p->slot_count ? P_ERROR : P_OK;
//}

int get_row_index(Page* p,size_t rowindex) {
	if (rowindex >= p->pdata.used_slot_size)
		return P_ERROR;
	return rowindex;
}

//������ù�init_file
void write_file_head(const FHead* p){
	FILE* fd_ = fopen(p->filename_, "rb+");
	fwrite(p->filehead, PageSize, 1, fd_);
	fflush(fd_);
	fclose(fd_);
}

FileHeadData* new_file_head_data(size_t pagecount, size_t rowslotcount, size_t rowlen){
	FileHeadData* fhg = mem_alloc(PageSize);
	fhg->page_count= pagecount;
	fhg->row_len = rowlen;
	fhg->row_slot_count = rowslotcount;
	fhg->page_used_size = 0;
	return fhg;
}

void init_file(FHead *fh){

	fclose(fopen(fh->filename_, "a"));
	//
	write_file_head(fh);

	//struct {
	//	size_t page_id;
	//	size_t used_slot_size;
	//	char slot_state_head[];
	//}*initdata;

	//size_t pagecount = fh->filehead->page_count;

	//size_t len = sizeof(*initdata) +
	//	fh->filehead->row_slot_count * sizeof(char);
	//
	//initdata = mem_calloc(1, len);

	//size_t offset = PageSize - len;

	//FILE* f = fopen(fh->filename_, "rb+");

	//fseek(f, PageSize, SEEK_CUR);

	//for (size_t i = 0; i < pagecount; i++) {
	//	fwrite(initdata, len, 1, f);
	//	initdata->page_id++;
	//	fseek(f, offset, SEEK_CUR);
	//}

	//mem_free(initdata);
	//fflush(f);
	//fclose(f);
}

FHead* new_file_head(const char* filename, FileHeadData* fhd){
	FHead* fh = mem_calloc(1,sizeof(*fh));
	LIST_INIT(&fh->head);
	fh->filename_ = filename;
	fh->filehead = fhd;
	//memset(fh->mem_page_bit_map, 0, PageCount*size_t);
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

int load_page(FHead* p,size_t id,Page* page){
	FILE* fd_;

	fd_ = fopen(p->filename_, "rb");

	fseek(fd_, (id + 1) * PageSize, SEEK_CUR);

	fread(&page->pdata, PageSize, 1, fd_);

	fclose(fd_);

	page->slot_count = p->filehead->row_slot_count;
	page->row_len = p->filehead->row_len;
	p->mem_page_bit_map[id] = page;
	return 1;
}

int store_page(Page* page,FHead* p){
	FILE* fd_;
	fd_ = fopen(p->filename_, "rb+");

	fseek(fd_, (page->pdata.page_id + 1) * PageSize, SEEK_CUR);

	fwrite(&page->pdata, PageSize, 1, fd_);

	fclose(fd_);
	//dict_del_entry_nofree(p->page_id_map, 
	//	page->pdata.page_id);
	return 1;
}

int file_get_not_full_page_id(FHead* f) {
	char* page_state_head = f->page_states;
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

int next_page_id(const FHead* p, size_t* index) {
	if (page_has_next(p, *index) == P_ERROR);
		return P_ERROR;
	if (p->page_states[*index] == P_EMPTY) {
		++(*index);
		return next_page_id(p, index);
	}
	return *index;
}

int page_state(FHead* f, size_t pid) {
		
}

Page* file_get_mem_page(FHead* f ,size_t pageid) {
	if (pageid >= f->filehead->page_count)
		return NULL;
	Page *p;
	if (p = f->mem_page_bit_map[pageid])
		return p;
}

Page* file_get_new_page(FHead* f ,size_t pageid) {
	Page *p = new_page(f->filehead->row_len, f->filehead->row_slot_count);
	load_page(f, pageid, p);
	f->mem_page_bit_map[pageid] = p;
	return p;
}

void set_page_full_state(FHead* p, int id,char state){
	p->page_states[id] = state;
}

Page* new_empty_page(void) {
	Page* p = mem_alloc(sizeof(Page) - 
		sizeof(PageData) + PageSize);
	LIST_INIT(&p->head);
	p->is_dirty = 0;
	p->row_offset_table = p->pdata.offset_table_and_rows;
	return p;
}

Page* new_page(size_t rowlen, size_t slot_count){
	Page* p = new_empty_page();
	p->row_len = rowlen;
	p->slot_count = slot_count;
	p->row_offset_table = p->pdata.offset_table_and_rows;
	p->rows_head = p->row_offset_table + slot_count;
	p->m_rows = mem_alloc(sizeof(size_t)*slot_count);
	return p;
}

void pagedata_init(PageData* pd){

}

void page_del(FHead* f, Page* p){
	f->mem_page_bit_map[p->pdata.page_id] = NULL;
	mem_free(p);
}

int page_add_row(Page* p, int index,const char* row){
	//if (index >= p->slot_count)
	//	return P_ERROR;
	//if (p->slot_state_head_ptr[index] == P_NOT_EMPTY)
	//	return page_add_row(p, row, index++);
	
	size_t offset = index* p->row_len;

	char* prow = p->rows_head + offset;

	memcpy(prow, row, p->row_len);
	p->pdata.used_slot_size++;
	p->slot_state_head_ptr[index] = P_NOT_EMPTY;
	return P_OK;
}

int page_get_free_slot(Page * p){
	for (int index = 0; p->slot_count; index++) {
		if (p->slot_state_head_ptr[index] = P_EMPTY)
			return index;
	}
	return P_ERROR;
}

int file_find_pid(FHead * f, const char* key){
	
	return 0;
}

char* page_get_row(Page * p, size_t index){
	return p->rows_head + index * p->row_len;
}

//int file_add_row(FHead* f,const char* row) {
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
//	if (++p->pdata.used_slot_size == p->slot_count)
//		f->page_states[p->pdata.page_id] = P_FULL;
//		
//	return P_OK;
//}

char* page_next_row(const Page* p,size_t* rowiter) {
	if (row_has_next(p, *rowiter) == P_ERROR)
		return NULL;

	if (p->slot_state_head_ptr[*rowiter] == P_EMPTY) {
		++(*rowiter);
		return page_next_row(p, rowiter);
	}
	return p->rows_head + (*rowiter)++ * p->row_len;
}

