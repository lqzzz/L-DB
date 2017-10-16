#ifndef PAGE_H
#define PAGE_H
#include<stdint.h>
#include"../BaseStruct/Dict.h"

#define PageSize 1024 
#define PageCount 16

#define P_ERROR -1
#define P_OK 0

#define P_EMPTY 0
#define P_NOT_EMPTY 1
#define P_FULL 2

typedef struct {
	size_t page_id;
	size_t used_slot_size;
	char slot_state_head;
	char row_set[];
}PageData;

typedef struct {
	size_t row_len;
	size_t slot_count;
	char* slot_state_head_ptr;
	PageData pdata;
}Page;

typedef struct{
	size_t row_slot_count;
	size_t row_len;
	size_t used_size;
	size_t page_count;
	char page_state_head[];
}FileHeadData;

typedef struct {
	char* filename_;
	Dict* page_id_map;
	FileHeadData* filehead;
}FHead;

FileHeadData* new_file_head_data(size_t pagecount,
	size_t rowslotcount,size_t rowlen);
FHead* new_file_head(char* filename, FileHeadData* fhd);
void write_file_head(FHead* fh);
FHead* read_file_head(char* filename, size_t rowlen, size_t rowslotcount);
void init_file(FHead* fh);

Page* new_page(size_t rowlen, size_t slot_count);
int load_page(Page* p, int index, FHead* fh);
int store_page(Page*, FHead*);
void page_init(Page* p, size_t rowlen, size_t slot_count);
int page_add_row(Page* p, size_t slot_iter, char* row);
void pagedata_init(PageData* pd);

Page* file_get_page_by_id(FHead* p, size_t pageid);
int file_get_not_full_page_id(FHead* p);
void set_page_full_state(FHead* p, int id,char state);
char* get_file_name(FHead* p);

char* page_next_row(Page* page, size_t *rowiter);
int next_page_id(FHead* filehead, size_t* pageid);

#endif // !PAGE_H

