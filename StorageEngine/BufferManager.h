#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include"Page.h"
typedef struct buffermanager* PBM;
typedef struct piter* PIterator;

PBM get_buffman(int DBid);
void new_buffermanager(int dbid);
void bm_add_raw_file_head(PBM bm, FHead filehead);
int buf_insert(PBM bm, const char* filename, const char* row);
PIterator get_table_iter(PBM bm, const char* filename);
const char* next_row(PIterator i);

//char* scan_table(char* tablename, *int pid, *int rid);
#endif // !BUFFERMANAGER_H

