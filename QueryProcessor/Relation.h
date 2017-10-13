#ifndef RELATION_H
#define RELATION_H
#include"../BaseStruct/Pair.h"
#include"../Catalog.h"
#include"../StorageEngine/Page.h"

typedef struct {

	struct rehead{
		Listhead head_;
		Pair schema_name;
		size_t rec_offset;
		size_t* max_len;
		size_t data_size;
		enum Tokentype data_type;
	}*rehead_list;

	int16_t rela_id;
	char* file_name;
	int16_t is_join;
	int16_t print_head_state;
	size_t row_iter;
	size_t* page_id;
	size_t data_len;
	Vector data_set;
	char* row_;
}Relation;

Relation* new_relation(Table* table);
Relation* rela_create(size_t datalen, int isjoin,struct rehead*,int v_size);
void rela_add_col(Relation* rela,Column* col);
Relation* get_relation(Table* table_name);

Relation* rela_loop_join(Relation* left,Relation* right);
struct rehead* rela_search_col(Relation* rela, Pair* schema);
size_t rela_get_max_len(Relation* rela,int colindex);
void rela_print(Relation* rela);
Relation* rela_or(Relation* left, Relation* right);
Relation* rela_filter(Relation* rela,DBnode* db, Pair* first, Pair* second, int opertype);

#endif // !RELATION_H
