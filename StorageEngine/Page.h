#ifndef PAGE_H
#define PAGE_H
#include<stdint.h>
#include"../BaseStruct/Vector.h"

#define PageSize 1024 
#define PageCount 16 

#define P_ERROR -1
#define P_OK 0

#define P_EMPTY '\0'
#define P_NOT_EMPTY '1'
#define P_FULL '2'

#define R_EMPTY '\0'

typedef struct file* FHead;
typedef const struct file* CFHead;

typedef struct p* Page;
typedef const struct p* CPage;

typedef struct mvccrows* MvccRows;

struct mvccrows{
	Listhead head;
	int create_verson;
	int del_verson;
	char* row;
};

struct p {

	struct phead{
		Listhead head;
		size_t row_len;
		size_t slot_count;
		int16_t is_dirty;
		MvccRows* arr_mrows;
		char* rows_head;
	};

	struct {
		size_t page_id;
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
		size_t page_used_size;
		size_t page_count;
		char page_states[];
	}info;
};

typedef struct {
	int pid;
	int rid;
	MvccRows value;
	FHead file;
	Page p;
}PIterator;

FHead* new_file_head(const char* filename,size_t pagecount,
	size_t rowslotcount,size_t rowlen);

//FHead new_file_head(char* filename, size_t pagecount,size );

void write_file_head(FHead fh);
FHead read_file_head(const char* filename, size_t rowlen, size_t rowslotcount);
void init_file(FHead fh);

Page new_empty_page(void);
Page new_page(size_t rowlen, size_t slot_count);
int load_page(FHead p, size_t id, Page page);
int store_page(Page, FHead);
void page_init(Page p, size_t rowlen, size_t slot_count);
//int page_add_row(Page p, size_t slot_index, const char* row);
void page_del(FHead f, Page p);
int page_add_row(Page p,int index, const char* row);
int page_get_empty_slot(Page p);
int file_find_pid(FHead f,const char* key);

char* file_get_row(FHead fh, size_t pageid, size_t rowindex);
char* page_get_row(const Page p, size_t index);

Page file_get_new_page(FHead p, size_t pageid);
Page file_get_mem_page(FHead f, size_t pageid);

int file_get_not_full_page_id(const FHead p);
void set_page_full_state(FHead p, int id,char state);
char* get_file_name(const FHead p);

int get_row_index(Page p, size_t rowindex);

char* page_next_row(const Page page, size_t *rowiter);
int next_page_id(const FHead filehead, size_t* pageid);

#endif // !PAGE_H

