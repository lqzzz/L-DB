#ifndef PAGE_H
#define PAGE_H
#include<stdint.h>
#include"../BaseStruct/Vector.h"

#define PageSize 1024 
#define PageCount 16

typedef struct page{
	char* filename;
	size_t row_len;
	size_t slot_count;

	size_t page_id;
	size_t used_slot_size;
	size_t* row_dir;
	char row_set[];

}Page;

char* page_next_row(Page* page, size_t *rowiter);
int page_add_row(Page* p, VectorIter* rowiter);

#endif // !PAGE_H

