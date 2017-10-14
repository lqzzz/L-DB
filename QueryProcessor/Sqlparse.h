#ifndef _TOKENIZER_H
#define _TOKENIZER_H
#include"SourceStream.h"
#include "Catalog.h"
int sql_parse(Arena* arena, DBnode *dbnode, Srcstream* stream);
#endif // !_TOKENIZER_H
 