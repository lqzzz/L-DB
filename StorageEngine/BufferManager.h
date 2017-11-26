#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include"../Catalog.h"
#include"Page.h"
typedef struct buffermanager* PBM;

//void page_fill(PBM bm, char* filename, VectorIter* rowiter);
PBM get_buffman(int DBid);
//PBM init_bm(int DBid);
void new_bufferManager(DBnode* db);
void bm_add_raw_file_head(PBM bm, FHead filehead);
char* get_next_row(PBM bm, const char* filename, int *pageiter, int *rowiter);
int buf_insert(PBM bm, const char* filename, const char* row);
Page buf_get_page(PBM bm, const char* filename, int pid);
//char* scan_table(char* tablename, *int pid, *int rid);
#endif // !BUFFERMANAGER_H

