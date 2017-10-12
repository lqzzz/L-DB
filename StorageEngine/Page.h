#ifndef PAGE_H
#define PAGE_H
#include<stdint.h>
#include"../BaseStruct/Vector.h"

#define PageSize 1024 
#define PageCount 16

#define P_ERROR -1
#define P_OK 0

#define P_EMPTY 0
#define P_NOT_EMPTY 1



typedef struct {
	size_t page_id;
	size_t used_slot_size;
	char slot_state_head;
	char row_set[];
}PageData;

typedef struct {
	size_t row_len;
	size_t slot_count;
	PageData pdata;
}Page;

//char* page_next_row(Page* page, size_t *rowiter);

int page_add_row(Page* p, size_t slot_iter, char* row);

#endif // !PAGE_H

