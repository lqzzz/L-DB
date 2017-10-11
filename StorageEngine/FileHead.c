#include<stdio.h>

#include"../BaseStruct/Vector.h"
#include "FileHead.h"


__inline int page_has_next(FHead* p, size_t index) {
	return index == p->page_count ? 0 : 1;
}

void write_file_head(FHead* p){
	FILE* fd_ = fopen(p->filename_, "rb+");

	fwrite(&p->page_count, sizeof(size_t), 1, fd_);

	fwrite(p->full_state_dir, sizeof(int16_t) * PageCount, 1, fd_);

	fwrite(p->page_dir, sizeof(size_t)*PageCount, 1, fd_);

	int i = 10;
	fwrite(&i, sizeof(int), 1, fd_);

	fflush(fd_);
	fclose(fd_);
}

FHead* new_file_head(char* filename, size_t rowlen, 
	size_t pagecount,int16_t* isfull, 
	size_t* pagedir,size_t rowslotcount) {

	FHead* pf_ = mem_alloc(sizeof(*pf_));
	pf_->filename_ = filename;
	pf_->row_len = rowlen;
	pf_->page_count = pagecount;
	pf_->full_state_dir = isfull;
	pf_->page_dir = pagedir;
	pf_->row_slot_count = rowslotcount;
	pf_->page_id_map = dict_create(NULL);	
	return pf_;
}

FHead* read_file_head(char* filename,size_t rowlen,size_t rowslotcount) {
	FILE* fd_;
	fd_ = fopen(filename, "rb");
	if (fd_ == NULL) return 1;

	//load page count
	fseek(fd_, 0, SEEK_CUR);
	size_t page_count = 0;
	fread(&page_count, sizeof(size_t),1, fd_);

	//load is full dir
	int16_t* full_state_dir = mem_alloc(sizeof(int16_t) * page_count);
	fread(full_state_dir, sizeof(int16_t), page_count, fd_);

	//load dir
	size_t *page_dir = mem_alloc(sizeof(size_t)*page_count);
	fread(page_dir, sizeof(size_t), page_count, fd_);

	fclose(fd_);
	return new_file_head(filename, rowlen, page_count, 
		full_state_dir, page_dir, rowslotcount);
}

int load_page(Page* page,int id,FHead* p){
	FILE* fd_;
	fd_ = fopen(p->filename_, "rb");
	size_t slot_count = p->row_slot_count;
	size_t row_len = p->row_len;
	fseek(fd_, id * PageSize + PageSize, SEEK_CUR);

	//load id
	size_t page_id = 0;
	fread(&page_id, sizeof(size_t), 1, fd_);

	//load used_count
	size_t used_slot = 0;
	fread(&used_slot, sizeof(size_t), 1, fd_);

	//load row dir
	size_t *row_dir = mem_alloc(sizeof(size_t) * slot_count);
	fread(row_dir, sizeof(size_t), slot_count, fd_);

	//load row set
	fread(page->row_set, row_len, slot_count, fd_);
	fclose(fd_);
	char* s = page->row_set;
	char* s1 = page->row_set + 10;
	page->filename = p->filename_;
	page->slot_count = slot_count;
	page->row_len = row_len;
	page->row_dir = row_dir;
	page->page_id = page_id;
	page->used_slot_size = used_slot;
	dict_add_entry(p->page_id_map, page_id, page);
	return 1;
}

int store_page(Page* page,FHead* p){
	FILE* fd_;
	fd_ = fopen(p->filename_, "rb+");
	fseek(fd_, (page->page_id + 1) * PageSize, SEEK_CUR);

	fwrite(&page->page_id, sizeof(size_t), 1, fd_);

	fwrite(&page->used_slot_size, sizeof(size_t), 1, fd_);

	fwrite(page->row_dir, sizeof(size_t), page->slot_count, fd_);

	fwrite(page->row_set, page->row_len, page->slot_count, fd_);
	fclose(fd_);
	dict_del_entry(p->page_id_map, page->page_id);
	return 1;
}

int get_not_full_page_id(FHead* f) {
	int16_t* full_state_dir = f->full_state_dir;
	size_t page_count = f->page_count;
	int page_id = -1;
	
	for (size_t i = 0; i < page_count; i++) 
		if (full_state_dir[i] != 1) {
			page_id = i;
			break;
		}
	//todo can't find 
	return page_id;
}

char * get_file_name(FHead* p){
	return p->filename_;
}



int next_page(FHead* p, size_t* index) {
	if (page_has_next(p, *index) == 0)
		return -1;
	if (p->full_state_dir[*index] == 0) {
		++(*index);
		return next_page(p, index);
	}
	return *index;
}

Page* get_page_ptr(FHead* p ,size_t pageid) {
	return dict_get_value(p->page_id_map, pageid);
}

//void grow_page(FHead* p) {
//	FILE* fd_ = fopen(p->filename_, "rb+");
//	
//}

void set_page_full_state(FHead* p, int id,int16_t state){
	p->page_dir[id] = state;
}
