#include "Pair.h"

Pair * pair_create(void * first, void * second,PairType *type){
	Pair *pair_ = mem_alloc(sizeof(Pair));
	pair_->piartype_ = type;
	pair_->first_ = first;
	pair_->second_ = second;
	return pair_;
}

PairType* pair_type_create(void(*first_free)(void*), 
							void(*second_free)(void*), 
							void*(*first_dup)(void*), 
							void*(*second_dup)(void*), 
							int16_t(*first_cmp)(void*, void*)){
	PairType *type_ = mem_alloc(sizeof(PairType));
	type_->first_dup = first_dup;
	type_->second_dup = second_dup;
	type_->first_free = first_free;
	type_->second_free = second_free;
	type_->first_cmp = first_cmp;
	return type_;
}

int16_t pair_cmp(Pair* p1,Pair* p2){
	return p1->piartype_->first_cmp(p1->first_, p2->first_);
}


