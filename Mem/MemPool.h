#ifndef __MEMPOOL_
#define __MEMPOOL_
#include<stdint.h>
#include"../BaseStruct/Listhead.h"
#define SIZE_ALINE(__n) if(__n&(sizeof(long)-1)) __n += sizeof(long)-(__n&sizeof(long)-1)

typedef struct Anode{
	Listhead list_;
	char chunk_[];
}Anode;

typedef struct {
	Anode* blocks_;
	size_t usedsize_;
}Arena;

typedef struct {
	void* (*m_alloc)(Arena* mpool,size_t size);
	void* (*m_calloc)(Arena* mpool,int num, size_t size);
	void* (*m_realloc)(Arena* mpool,size_t size);
	void (*m_free)(void* ptr);
}Alloctor;

Arena* new_arena();
void free_arena(Arena* a);
void* a_realloc(Arena* a, void* ptr, size_t newsize);
void* a_calloc(Arena* a, int num, size_t size);
void* a_alloc(Arena* a, size_t size);
void a_free(Arena* a, void* ptr);
size_t a_get_size(Arena* a);

void* mem_alloc(size_t size);
void mem_free(void *ptr);
void* mem_calloc(int num,size_t size);
void* mem_realloc(void*,size_t);
size_t malloc_used_memory(void);
void mem_alloc_enable_thread_safeness(void);
#endif // !__MEMPOOL_
