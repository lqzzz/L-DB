#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"../pthread.h"
#include"MemPool.h"
#pragma comment(lib,"pthreadVC2.lib")

static size_t used_memory = 0;
static int mem_alloc_thread_safe = 0;
pthread_mutex_t used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;

#define PREFIX_SIZE sizeof(size_t)

#define increment_used_memory(__n) do { \
    size_t _n = (__n); \
	SIZE_ALINE(_n);\
    if (mem_alloc_thread_safe) { \
        pthread_mutex_lock(&used_memory_mutex);  \
        used_memory += _n; \
        pthread_mutex_unlock(&used_memory_mutex); \
    } else { \
        used_memory += _n; \
    } \
} while(0)

#define decrement_used_memory(__n) do { \
    size_t _n = (__n); \
	SIZE_ALINE(_n);\
    if (mem_alloc_thread_safe) { \
        pthread_mutex_lock(&used_memory_mutex);  \
        used_memory -= _n; \
        pthread_mutex_unlock(&used_memory_mutex); \
    } else { \
        used_memory -= _n; \
    } \
} while(0)

void free_node(Arena* a, Anode* node) {
	if (node->list_.next_ == node)
		return;
}

Arena* new_arena() {
	size_t size_ = sizeof(Arena);
	SIZE_ALINE(size_);
	Arena *a_ = mem_alloc(size_);
	a_->usedsize_ = size_ + PREFIX_SIZE;
	return a_;
}

void free_arena(Arena * a){
	
}

inline Anode* new_node(size_t size) {
	size_t size_ = sizeof(size);
	SIZE_ALINE(size_);
	Anode* node_ = mem_alloc(size);
	LIST_INIT(&node_->list_);
	return node_;
}

inline void arena_add_node(Arena* a, Anode* node) {
	if (a->blocks_)
		LIST_ADD_TAIL(&a->blocks_->list_, &node->list_);
	else 
		a->blocks_ = node;	
}

void* a_alloc(Arena* a, size_t size) {
	size_t size_ = size;
	SIZE_ALINE(size_);
	size_t node_size = size_ + sizeof(Anode);
	Anode* node_ = mem_alloc(node_size);
	LIST_INIT(&node_->list_);
	arena_add_node(a, node_);
	a->usedsize_ += node_size + PREFIX_SIZE;
	return node_->chunk_;
}

void a_free(Arena * a, void * ptr){
	Anode* node = (char*)ptr - sizeof(Anode);
	size_t size_ = *((size_t*)((char*)ptr - PREFIX_SIZE));
	
}

size_t a_get_size(Arena * a){

	return a->usedsize_;
}

void* a_calloc(Arena* a, int num, size_t size) {
	size_t node_size = size;
	SIZE_ALINE(node_size);
	size_t sum_len = num*node_size + sizeof(Anode);
	Anode* node_ = mem_alloc(sum_len);
	memset(node_->chunk_, 0, num*node_size);
	arena_add_node(a, node_);
	a->usedsize_ += sum_len + PREFIX_SIZE;
	return node_->chunk_;
}

void* a_realloc(Arena* a,void* ptr,size_t newsize) {
	Anode* realnode;
	size_t oldsize;
	Anode* newnode;
	if (ptr == NULL) 
		return a_alloc(a, newsize);
	realnode = (char*)ptr - sizeof(Anode);
	oldsize = *((size_t*)((char*)realnode - PREFIX_SIZE));

	Listhead* prev_ = realnode->list_.prve_;
	Listhead* next_ = realnode->list_.next_;
	size_t size_ = newsize + sizeof(Anode);
	SIZE_ALINE(size_);
	newnode = mem_realloc(realnode, size_);
	prev_->next_ = &newnode->list_;
	next_->prve_ = &newnode->list_;

	a->usedsize_ -= oldsize;
	a->usedsize_ += newsize + PREFIX_SIZE;
	return newnode->chunk_;
}

static void malloc_oom(size_t size) {
	fprintf(stderr, "malloc: Out of memory trying to allocate %zu bytes\n",
		size);
	fflush(stderr);
	abort();
}

void* mem_calloc(int num, size_t size) {
	size_t node_size = size;
	SIZE_ALINE(node_size);
	size_t sum_len = num*node_size;

	void *ptr = malloc(sum_len + PREFIX_SIZE);
	if (!ptr) malloc_oom(sum_len);
	memset((char*)ptr+PREFIX_SIZE, 0, sum_len);
	*((size_t*)ptr) = sum_len;
	increment_used_memory(sum_len+PREFIX_SIZE);
	return (char*)ptr + PREFIX_SIZE;
}

void* mem_realloc(void* ptr,size_t size){
	void *realptr;
	size_t oldsize;
	void *newptr;
	if (ptr == NULL) return mem_alloc(size);
	realptr = (char*)ptr-PREFIX_SIZE;
	oldsize = *((size_t*)realptr);
	newptr = realloc(realptr, size + PREFIX_SIZE);
	if (!newptr) malloc_oom(size);
	*((size_t*)newptr) = size;
	decrement_used_memory(oldsize);
	increment_used_memory(size);
	return (char*)newptr+PREFIX_SIZE;
}


void* mem_alloc(size_t size){
	void *ptr = malloc(size + PREFIX_SIZE);
	if (!ptr) malloc_oom(size);
	*((size_t*)ptr) = size;
	increment_used_memory(size + PREFIX_SIZE);
	return (char*)ptr+PREFIX_SIZE;
}

void mem_free(void* ptr) {
	if (ptr == NULL) return;
	void* realptr = (char*)ptr - PREFIX_SIZE;
	size_t oldsize = *((size_t*)realptr);
	decrement_used_memory(oldsize + PREFIX_SIZE);
	free(realptr);
}

size_t malloc_used_memory(void) {
	size_t um;
	if (mem_alloc_thread_safe) pthread_mutex_lock(&used_memory_mutex);
	um = used_memory;
	if (mem_alloc_thread_safe) pthread_mutex_unlock(&used_memory_mutex);
	return um;
}

void mem_alloc_enable_thread_safeness(void) {
	mem_alloc_thread_safe = 1;
}
