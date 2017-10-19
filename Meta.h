#ifndef _META_H
#define _META_H
#include "Catalog.h"

DBnode* init_sys_data();

void db_show(char* name);
void table_show(DBnode* db,char* name);
void col_show(Table* t, char* name);


#endif // !_META_H

