
#include"Sqlparse.h"

QueryNode* create_join(Vector* from) {
	Table* table_;
	QueryNode* node = NULL;	
	
	VECTOR_FOREACH(table_, from, 	
		QueryNode* new_node = mem_calloc(1, sizeof(*new_node));
		size_t len = strlen(table_->name_) + 1;
		char* table_name = mem_alloc(len);
		memcpy(table_name, table_->name_, len);

		new_node->nodetype = JOIN;
		Pair* p = new_pair;
		PairSetFirst(p, table_name);
		new_node->left_opand = p;
		if (node == NULL)
			node = new_node;
		else {
			new_node->next_ = node;
			node = new_node;
		}
	);
	return node;
}
