#include<stdio.h>
#include"../Mem/MemPool.h"
#include"Page.h"

int page_add_row(Page* p,VectorIter* rowiter) {
	size_t used_solt = p->used_slot_size;
	size_t slot_count = p->slot_count;
	size_t row_len = p->row_len;
	size_t* row_dir = p->row_dir;

	char* row_set = p->row_set;
	while (used_solt != slot_count) {
		if (vector_has_next(rowiter) == 0)
			return 0;
		memcpy(row_set + (row_dir[used_solt++]),
			vector_next(rowiter), row_len);
		p->used_slot_size++;
	}
	return 1;
}

__inline int row_has_next(Page* p,size_t rowindex) {
	return rowindex == p->used_slot_size ? 0 : 1;
}

char* page_next_row(Page* p,size_t* rowiter) {
	return row_has_next(p, *rowiter) ?
		memcpy(mem_alloc(p->row_len),
			p->row_set + p->row_dir[(*rowiter)++],p->row_len) 
		: NULL;
}

