#include"TestFrameWork.h"
#include"../QueryProcessor/Scanner.h"
#include<string.h>
#include"../Mem/MemPool.h"
#include"../Meta.h"
char* sqlstr = "create table t1(col1 int, col2 char, col3 char(10))";
char* s;
int c = 0;
int l = 0;

static void print_token(Token* t) {
	if(t->token_type == INT)
		printf("Type : %d,Value : %d\n", t->token_type, t->value_);
	else
		printf("Type : %d,Value : %s\n", t->token_type, t->value_);
}

void scanner_test_init(void) {
	init_key_word();
	size_t len = strlen(sqlstr) + 1;
	s = mem_alloc(len);
	memcpy(s, sqlstr, len);
}

void scanner_test(void) {
	scanner_test_init();

	char errmsg[100];
	Token* t = scanner(errmsg, &s, &c, &l);
	Token* c;
	LIST_FOREACH(c, t,
		print_token(c);
		);
}