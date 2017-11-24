#include "dict.h"
#include<stdio.h>

#define DICT_EQ 0

static unsigned long dict_next_power(unsigned long size);
static void dict_init(Dict* d,DictType* dicttype);
static int dict_expand_ifneeded(Dict *d);
static void dict_reset(DictHt* ht);
static DictEntry** dict_get_entry(const Dict * dict, const void * key);
static int dict_resize_ifneeded(Dict* d);
static int dict_del_gen(Dict * dict, const void * key,int flag);

int dict_expand_ifneeded(Dict *d) {
	if (d->ht_.size_ == 0) 
		return dict_expand(d, DICT_HT_INITIAL_SIZE);
	if (d->ht_.used_ > d->ht_.size_ && d->ht_.used_ / d->ht_.size_ >= 2)
		return dict_expand(d, d->ht_.size_ * 2);
	return DICT_OK;
}

unsigned long dict_next_power(unsigned long size) {
	unsigned long i = DICT_HT_INITIAL_SIZE;
	while (1) {
		if (i >= size)
			return i;
		i *= 2;
	}
}

void dict_reset(DictHt* ht) {
	ht->sizemask_ = 0;
	ht->table_ = NULL;
	ht->sizemask_ = 0;
	ht->used_ = 0;
	ht->size_ = 0;
}

void dict_init(Dict* d,DictType* type) {
	dict_reset(&d->ht_);
	d->type_ = type;
}

int dict_del_gen(Dict * dict, const void * key,int flag) {
	DictEntry** ptr_ = dict_get_entry(dict, key);
	if (*ptr_ == NULL)
		return DICT_ERR;
	DictEntry* curr_ = *ptr_;
	*ptr_ = curr_->next_;
	if (flag) {
		DictFreeKey(dict, curr_);
		DictFreeValue(dict, curr_);
	}
	mem_free(curr_);
	dict->ht_.used_--;
	return dict_resize_ifneeded(dict);
}

int dict_del_entry(Dict * dict, const void * key) {
	return dict_del_gen(dict, key, 1);
}

int dict_del_entry_nofree(Dict * dict, const void * key){
	return dict_del_gen(dict, key, 0);
}

int dict_resize_ifneeded(Dict * dict){
	if (dict->ht_.size_>64 &&
		dict->ht_.size_ > dict->ht_.used_ &&
		dict->ht_.size_ / dict->ht_.used_ >= 2) {
		int minnum = dict->ht_.used_;
		if (minnum < DICT_HT_INITIAL_SIZE)
			minnum = DICT_HT_INITIAL_SIZE;
		return dict_expand(dict, minnum);
	}
	return DICT_OK;
}

unsigned int dict_str_hashfunction(const char * str){
	if (str == NULL)
		return 0 & 0x7FFFFFFF;
	unsigned int seed = 5381;
	unsigned int hash = 0;
	while (*str)
		hash = hash * seed + (*str++);
	return (hash & 0x7FFFFFFF);
}

unsigned int dict_int_hashfunction(unsigned int key){
	key += ~(key << 15);
	key ^=  (key >> 10);
	key +=  (key << 3);
	key ^=  (key >> 6);
	key += ~(key << 11);
	key ^=  (key >> 16);
	return key;
}

DictEntry** dict_get_entry(const Dict *dict, const void *key){
	if (dict_expand_ifneeded(dict) == DICT_ERR)
		return NULL;
	int index_ = DictHashKey(dict,key) % dict->ht_.size_;
	DictEntry **curr_ = &dict->ht_.table_[index_];
	while (*curr_) {
		if (DictMatchHashKey(dict, (*curr_)->key_, key) == DICT_EQ)
			break;
		curr_ = &(*curr_)->next_;
	}
	return curr_;
}

Dict* new_dict(DictType *Type){
	Dict *dict_ = mem_alloc(sizeof(Dict));
	dict_init(dict_, Type);
	return dict_;
}

Dict * new_dict_len(DictType * Type, unsigned int len){
	Dict* d = new_dict(Type);
	dict_expand(d, len);
	return d;
}

int dict_expand(Dict * d, unsigned long size){
	DictHt ht;
	if (d->ht_.used_ > size)
		return DICT_ERR;
	unsigned long realsize_ = dict_next_power(size);
	ht.size_ = realsize_;
	ht.sizemask_ = realsize_ - 1;
	ht.table_ = mem_calloc(realsize_, sizeof(DictEntry*));
	ht.used_ = 0;

	if (d->ht_.table_ == NULL) {
		d->ht_ = ht;
		return DICT_OK;
	}

	for (unsigned int i = 0; i < d->ht_.size_; i++) {
		if (d->ht_.table_[i] == NULL)
			continue;
		DictEntry* h = d->ht_.table_[i];
		while (h) {
			DictEntry* nexte = h->next_;
			unsigned int index_ = DictHashKey(d, h->key_) & ht.sizemask_;

			h->next_ = ht.table_[index_];
			ht.table_[index_] = h;

			h = nexte;
		}
	}

	mem_free(d->ht_.table_);
	ht.used_ = d->ht_.used_;
	d->ht_ = ht;
	return DICT_OK;
}

int dict_add_entry(Dict* dict,void* key,void* value){
	DictEntry **ptr_ = dict_get_entry(dict,key);
	if (*ptr_) 
		return DICT_ERR;
	DictEntry *entry_ = mem_alloc(sizeof(DictEntry));
	DictSetHashKey(dict, entry_, key);
	DictSetHashValue(dict, entry_,value);
	entry_->next_ = NULL;
	*ptr_ = entry_;
	dict->ht_.used_++;
	return DICT_OK;
}

void* dict_get_value(const Dict *dict, const void * key){
	DictEntry *target_ = *dict_get_entry(dict, key);
	return target_ ? target_->value_ : NULL;
}
