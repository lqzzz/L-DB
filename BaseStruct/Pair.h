#ifndef  _PAIR_H
#define  _PAIR_H
#include"../Mem/MemPool.h"
#define PAIR_INIT(p) memset(p,0,sizeof(Pair))
#define PAIR_CLEAR(p) do{\
	if(p == NULL) break;\
	if ((p)->piartype_) {\
		(p)->piartype_->first_free((p)->first_);\
		(p)->piartype_->second_free((p)->second_);\
	}else {\
		mem_free((p)->first_);\
		mem_free((p)->second_);\
	}\
}while(0)
#define PairGetFirst(pair_) ((pair_)->first_)
#define PairGetSecond(pair_) ((pair_)->second_)
#define PairMatch(pair_,key_) ((pair_)->piartype_->first_cmp(key_,pair_->first_))
#define PairSetFirst(pair,value)((pair)->first_ = value)
#define PairSetSecond(pair,value)((pair)->second_ = value)
#define PAIR_FREE(p) PAIR_CLEAR(p); mem_free(p)
#define new_pair mem_calloc(1,sizeof(Pair))
typedef struct PairType{
	void(*first_free)(void*);
	void(*second_free)(void*);
	void*(*first_dup)(void*);
	void*(*second_dup)(void*);
	int16_t(*first_cmp)(void*, void*);
}PairType;
typedef struct Pair {
	void* first_;
	void* second_;
	PairType *piartype_;
}Pair;
Pair* pair_create(void* first, void* second, PairType* type);
PairType* pair_type_create(void(*first_free)(void*),
						   void(*second_free)(void*),
						   void*(*first_dup)(void*),
						   void*(*second_dup)(void*),
						   int16_t(*first_cmp)(void*, void*));
int16_t pair_cmp(Pair* p1,Pair* p2);
#endif // ! _PAIR_H

