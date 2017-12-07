#ifndef PAGE_H
#define PAGE_H
#include<stdint.h>
#include"../BaseStruct/Listhead.h"
#define PageSize 1024 
#define PageCount 16 

#define P_ERROR -1
#define P_OK 0

#define P_EMPTY '\0'
#define P_NOT_EMPTY '1'
#define P_FULL '2'

#define R_EMPTY '\0'
#define R_NOT_EMPTY '1'

typedef struct file* FHead;
typedef struct p* Page;

struct p {

	struct phead{
		Listhead head;
		const FHead f;
		size_t page_id;
		size_t row_len;
		size_t slot_count;
		int16_t is_dirty;
		char* rows_head;
	};

	struct {
		size_t used_slot_size;
		char slot_states[];
	}info;
};

struct file {

	struct filehead {
		Listhead head;
		char* filename_;
		Page* mem_page_bit_map;
	};

	struct {
		size_t row_slot_count;
		size_t row_len;
		size_t page_count;
		char page_states[];
	}info;
};

FHead* new_file_head(const char* filename,
	size_t rowslotcount,size_t rowlen);

void write_file_head(FHead fh);

FHead read_file_head(const char* filename, size_t rowlen, 
	size_t rowslotcount);

void init_file(FHead fh);

Page file_new_page(FHead f,size_t pid);

Page load_page(FHead p, size_t id);

int store_page(Page, FHead);

int page_free(Page p);

int page_del(FHead f, Page p);

int page_add_row(FHead f, Page p, const char* row);

int page_get_empty_slot(Page p);

Page file_get_mem_page(FHead f, size_t pageid);

int file_get_insert_pid(const FHead p);

char* get_file_name(const FHead p);

int get_row_index(Page p, size_t rowindex);

#endif // !PAGE_H

