#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include"../BaseStruct/Vector.h"
#include"../Catalog.h"
#include"Page.h"

typedef struct bm BufferManager;

void page_fill(BufferManager* bm, char* filename, VectorIter* rowiter);

BufferManager* get_buffman(int DBid);
//BufferManager* init_bm(int DBid);
void new_bufferManager(DBnode* db);
void bm_add_file_head(int DBid, FHead* filehead);
char* get_next_row(BufferManager* bm, char* filename, size_t *pageiter, size_t *rowiter);
char* scan_table(char* tablename, *int pid, *int rid);
#endif // !BUFFERMANAGER_H

