#ifndef __DICT_H
#define __DICT_H
#include<string.h>
#include<assert.h>
#include"../Mem/MemPool.h"
#define DICT_HT_INITIAL_SIZE     4
#define DICT_OK 1
#define DICT_ERR -1
#define HT_SIZE 97
typedef struct DictEntry {
	void *key_;
	void *value_;
	struct DictEntry *next_;
}DictEntry;

typedef struct DictHt {
	unsigned int size_;
	unsigned int sizemask_;
	unsigned int used_;
	DictEntry **table_;
}DictHt;

typedef struct DictType {
	void *(*key_dup)(const void* key);
	void *(*value_dup)(const void* value);
	int(*key_match)(const void* key1, const void* key2);
	int(*value_destructor)(void *value);
	int(*key_destructor)(void *key);
	unsigned int(*hashfunction)(const void* key);
}DictType;

typedef struct Dict {
	DictType *type_;
	DictHt ht_;
}Dict;

#define DictHashKey(d,key) (d)->type_->hashfunction(key) 

#define DictSetHashValue(d, entry, _value_) do { \
    if ((d)->type_ && (d)->type_->value_dup) \
        entry->value_ = (d)->type_->value_dup(_value_); \
    else \
        entry->value_ = (_value_); \
} while(0)

#define DictSetHashKey(d, entry,_key_) do { \
    if ((d)->type_&&(d)->type_->key_dup) \
        entry->key_ = (d)->type_->key_dup(_key_); \
    else \
        entry->key_ = (_key_); \
} while(0)

#define DictFreeKey(d,entry) do { \
	if((d)->type_&&(d)->type_->key_destructor) \
		(d)->type_->key_destructor((entry)->key_); \
	else \
		mem_free((entry)->key_); \
} while(0)

#define DictFreeValue(d,entry) do { \
	if((d)->type_&&(d)->type_->value_destructor) \
		(d)->type_->value_destructor((entry)->value_); \
	else \
		mem_free((entry)->value_); \
}while(0)

#define DictMatchHashKey(d,key_1,key_2)\
	(((d)->type_&&((d)->type_->key_match)) ? \
		(d)->type_->key_match(key_1,key_2) : \
		(key_1 == key_2))

Dict* new_dict(DictType *Type);
Dict* new_dict_len(DictType *Type,unsigned int len);
int dict_expand(Dict* d,unsigned long size);
int dict_add_entry(Dict *dict, void* key, void *value);
void* dict_get_value(const Dict * dict, const void * key);
int dict_del_entry(Dict* dict, const void* key);
int dict_del_entry_nofree(Dict* dict, const void* key);

unsigned int dict_str_hashfunction(const char* str);
unsigned int dict_int_hashfunction(unsigned int str);

#endif /* !__DICT_H */