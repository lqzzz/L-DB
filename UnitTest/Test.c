#include <stdio.h>
#include "TestFrameWork.h"
#include <string.h>

int main_ret = 0;
int test_count = 0;
int test_pass = 0;

//int test_strcmp(const char* str1, const char* str2) {
//	return strcmp(str1, str2) == 0 ? 1 : 0;
//}

void dict_test(void);
void page_test(void);
void meta_test(void);
void scanner_test(void);
void sqltest(void);

int main(void) {
	scanner_test();
	dict_test();
	page_test();
	meta_test();
	sqltest();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
}



