#ifndef _LISTHEAD_H
#define _LISTHEAD_H
#include<stddef.h>
#define LIST_INIT(head) ((Listhead*)head)->next_ = head; ((Listhead*)head)->prve_ = head
#define LIST_GET_NEXT(head) ((Listhead*)head)->next_
#define LIST_GET_PRVE(head) ((Listhead*)head)->prve_
#define LIST_MOVE_NEXT(head) *(head) = ((Listhead*)*(head))->next_
#define LIST_ENTRY(ptr,type,member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))  
#define LIST_ADD_TAIL(head,newnode)do{\
	(newnode)->prve_ = (head)->prve_;\
	(newnode)->next_ = head;\
	(head)->prve_->next_ = newnode;\
	(head)->prve_ = newnode;\
}while(0)

#define LIST_JOIN(first,second)do{\
	((Listhead*)first)->prve_->next_ = second;\
	((Listhead*)first)->prve_ = ((Listhead*)second)->prve_;\
	((Listhead*)second)->prve_->next_ = first;\
	((Listhead*)second)->prve_ = ((Listhead*)first)->prve_;\
}while(0)

#define LIST_FOREACH(curr_node,listhead,action) do{\
	Listhead *list_head = listhead;\
	Listhead *curr_ = listhead;\
	do{\
		curr_node = curr_;\
		action\
	}while((LIST_MOVE_NEXT(&curr_)) != list_head);\
}while(0)

#define LIST_DEL_ALL(delnode,listhead,delaction)do{\
	Listhead *head_ = listhead;\
	Listhead *next_= (listhead)->next_;\
	while (next_ != head_) {\
		delnode = next_;\
		LIST_MOVE_NEXT(&next_);\
		delaction\
	}\
	delnode = head_;\
	delaction\
}while(0)

typedef struct Listhead {
	struct Listhead *next_, *prve_;
}Listhead;

void list_del_all(Listhead* node, void del(void *));
void* list_search(Listhead *listhead, void* key, int cmp(void*, void*));

#endif // !_LISTHEAD_H
