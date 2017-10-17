#include "obj.h"
#include "MemPool.h"
obj* new_obj(void* data) {
	obj* o = mem_alloc(sizeof(obj));
	o->data_ = data;
	o->ref_count = 0;
	return o;
}

