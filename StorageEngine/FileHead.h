#ifndef FILEHEAD_H
#define FILEHEAD_H
#include<stdint.h>

#include"Page.h"
#include"../BaseStruct/dict.h"
#include"../BaseStruct/Listhead.h"

typedef struct {
	char* filename_;
	Dict* page_id_map;
};

typedef struct filehead {
	char* filename_;
	size_t row_slot_count;
	size_t row_len;
	size_t used_size;
	size_t page_count;
	int16_t* full_state_dir;//0 is empty, 1 is full, 2 is has row and not full
	size_t* page_dir; 

	Dict* page_id_map;
}FHead;

void write_file_head(FHead*);

FHead* read_file_head(char* filename, size_t rowlen, size_t rowslotcount);

FHead* new_file_head(char* filename, size_t rowlen,
	size_t pagecount,int16_t* isfull, size_t* pagedir, 
	size_t rowslotcount);

int next_page(FHead* filehead, size_t* pageid);
Page* get_page_ptr(FHead* p, size_t pageid);
int get_not_full_page_id(FHead* p);
int load_page(Page*, int, FHead*);
int store_page(Page*, FHead*);

char* get_file_name(FHead* p);
void set_page_full_state(FHead* p, int id,int16_t state);

#endif // !FILEHEAD_H
