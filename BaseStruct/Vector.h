#ifndef _VECTOR_H
#define _VECTOR_H
#include"../Mem/MemPool.h"
#include<string.h>
#define VITER_START_HEAD 0
#define VITER_START_TAIL 1
#define INIT_LEN 4

typedef struct Vector{
	size_t  usedsize_;
	size_t  freesize_;
	void*(*dup_)(void*);
	void(*free_)(void*);
	int32_t(*comp_)(void*, void*);
	void **vector_;
}Vector;

typedef struct VectorIter{
	size_t index_;
	size_t end_index;
	void **value_;
}VectorIter;

#define VECTOR_FOREACH(var,vec,action)do{\
	size_t _len = (vec)->usedsize_;\
	void** _vector = (vec)->vector_;\
	for(size_t _i = 0; _i<_len;_i++){\
		var = _vector[_i];\
		action\
	}\
}while(0)

#define VECTOR_CLEAR_VAL(v) do{\
	size_t len_ = (v)->usedsize_;\
	if ((v)->free_)\
		for (size_t i = 0; i < len_; ++i)\
			(v)->free_((v)->vector_[i]);\
	else\
		for (size_t i = 0; i < len_; ++i)\
			mem_free((v)->vector_[i]);\
}while(0)

#define VECTOR_CLEAR_NODE(v) mem_free((v)->vector_) 
#define VECTOR_RELEASE(v) VECTOR_CLEAR_VAL(v);VECTOR_CLEAR_NODE(v);mem_free(v)
#define VECTOR_PUSHBACK(v,value) vector_insert((v)->usedsize_,v,value) 
#define VECTOR_GET_VALUE(v,index) (v)->vector_[index]
#define VECTOR_RESET(v) memset((v)->vector_, 0, (v)->usedsize_ + (v)->freesize_)
#define	VECTOR_GROW(v,len)do{\
	size_t v_used_size = (v)->usedsize_;\
	size_t v_len = len;\
	Vector* v_ = v;\
	v_->vector_ = mem_realloc(v_->vector_, (v_used_size + v_len) * sizeof(size_t));\
	v_->freesize_ += v_len;\
	v_->vector_ = memset(v_->vector_ + v_used_size, 0, v_->freesize_ * sizeof(size_t));\
}while(0)

#define VECTOR_INIT(v,len) memset(v,0,sizeof(Vector)); VECTOR_GROW((v),len)
#define VECTOR_HAS_ELEM(v) (v)->usedsize_ ? 1:0
#define VECTOR_GET_SIZE(v) (v)->usedsize_
#define VectorGetFreeSize(v) ((v)->freesize_)
#define VectorGetUsedSize(v) ((v)->usedsize_)
#define VectorGetDupMethod(v) ((v)->dup_)
#define VectorGetMatchMethod(v) ((v)->comp_)
#define VectorGetFreeMethod(v) ((v)->free_)

#define VectorSetDupMethod(v,dup) ((v)->dup_ = dup)
#define VectorSetMatchMethod(v,comp) ((v)->comp_ = comp)
#define VectorSetFreeMethod(v,free) ((v)->free_ = free)

#define VECTOR_INIT_ITER(v,iter) \
	(iter)->value_ = &(v)->vector_[0];\
	(iter)->index_ = 0;\
	(iter)->end_index = (v)->usedsize_\

Vector* vector_create_len(int count);
Vector* vector_create();
int32_t vector_init_v(Vector* dest, Vector *vsrc);
int32_t vector_insert(size_t index, Vector *v, void* value);
int32_t vector_copy(Vector* vdest, Vector* vsrc);
Vector* vector_value_copy(Vector *vsrc);
VectorIter* vector_get_iter(Vector* v);
void vector_init_iter(Vector*, VectorIter*);
VectorIter* vector_get_end(Vector* v);
void vector_release_iter(VectorIter *iter);
void* vector_pop(Vector * v);
int32_t vector_eq_iter(VectorIter *begin, VectorIter *end);

Vector* vector_filter(Vector* v);
void vector_sort(Vector* v);
int32_t vector_search(Vector* v,void* key,int32_t(*comp_)(void*, void*));
int16_t vector_has_next(VectorIter* iter);
void* vector_next(VectorIter* iter);

#endif // !_VECTOR_H

