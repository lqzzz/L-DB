#include<stdio.h>
#include"../Mem/MemPool.h"
#include"Page.h"
#include<string.h>
#include<assert.h>

int get_row_index(Page p,size_t rowindex) {
	if (rowindex >= p->info.used_slot_size)
		return P_ERROR;
	return rowindex;
}

void write_file_head(FHead f){
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

FHead new_empty_file() {
	FHead fh = mem_alloc(sizeof(struct filehead) + PageSize);
	LIST_INIT(&fh->head);
	fh->info.page_count = 0;
	return fh;
}

FHead* new_file_head(const char* filename,
	size_t rowslotcount,size_t rowlen){
	FHead fh = new_empty_file();
	fh->filename_ = filename;
	fh->info.row_len = rowlen;
	fh->info.row_slot_count = rowslotcount;
	fh->mem_page_bit_map = mem_calloc(1, sizeof(Page) * fh->info.page_count);
	return fh;
}


FHead read_file_head(const char* filename){
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

Page load_page(FHead f,size_t pid){
	FILE* fd_;
	Page p = file_new_page(f,pid);

	fd_ = fopen(f->filename_, "rb");

	fseek(fd_, (pid + 1) * PageSize, SEEK_CUR);

	fread(&p->info, PageSize, 1, fd_);

	fclose(fd_);

	return p;
}

int store_page(Page page,FHead f){
	FILE* fd_;
	fd_ = fopen(f->filename_, "rb+");

	fseek(fd_, (page->page_id + 1) * PageSize, SEEK_CUR);

	fwrite(&page->info, PageSize, 1, fd_);

	fclose(fd_);

	return 1;
}

int page_free(Page p){
	p->head.next_ = p->head.prve_->next_;
	p->head.prve_ = p->head.prve_;
	p->f->mem_page_bit_map[p->page_id] = NULL;
	mem_free(p);
	return P_OK;
}

int file_get_insert_pid(FHead f) {
	for (size_t i = 0; i < f->info.page_count; i++) 
		if (f->info.page_states[i] != P_FULL) 
			return i;
	return P_ERROR;
}

char * get_file_name(FHead p){
	return p->filename_;
}

Page file_get_mem_page(FHead f ,size_t pageid) {
	return  f->mem_page_bit_map[pageid];
}

Page file_new_page(FHead f,size_t pid){
	Page p = mem_calloc(1, sizeof(struct phead) + PageSize);
	LIST_INIT(&p->head);
	p->slot_count = f->info.row_slot_count;
	p->row_len = f->info.row_len;
	p->rows_head = p->info.slot_states + p->slot_count * sizeof(char);
	p->f = f;
	p->page_id = pid;
	f->mem_page_bit_map = mem_realloc(f->mem_page_bit_map, sizeof(Page) * ++f->info.page_count);
	f->mem_page_bit_map[pid] = p;
	return p;
}

int page_add_row(Page p,size_t index,const char* row){
	//int index = 0;
	//for (; p->slot_count; index++)
	//	if (p->info.slot_states[index] == R_EMPTY)
	//		break;

	size_t offset = index* p->row_len;

	char* prow = p->rows_head + offset;
	memcpy(prow, row, p->row_len);
	p->info.slot_states[index] = R_NOT_EMPTY;

	if (p->slot_count == ++p->info.used_slot_size)
		p->f->info.page_states[p->page_id] = P_FULL;

	p->is_dirty = 1;

	return P_OK;
}

int page_get_empty_slot(Page p){
	for (int index = 0; p->slot_count; index++) {
		if (p->info.slot_states[index] == R_EMPTY)
			return index;
	}
	return P_ERROR;
}

char* page_get_row(Page p, size_t rid){
	return p->rows_head + rid * p->row_len;
}
