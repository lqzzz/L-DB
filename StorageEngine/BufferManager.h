#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include"../Catalog.h"
#include"Page.h"
typedef struct bm BufferManager;

//void page_fill(BufferManager* bm, char* filename, VectorIter* rowiter);
BufferManager* get_buffman(int DBid);
//BufferManager* init_bm(int DBid);
void new_bufferManager(DBnode* db);
void bm_add_raw_file_head(int DBid, FHead* filehead);
char* get_next_row(BufferManager* bm, const char* filename, int *pageiter, int *rowiter);
int buf_insert(BufferManager* bm, const char* filename, const char* row);
Page* buf_get_page(BufferManager* bm, const char* filename, int pid);
//char* scan_table(char* tablename, *int pid, *int rid);
#endif // !BUFFERMANAGER_H

