#ifndef __TESTFRAMEWORK_H
#define __TESTFRAMEWORK_H
#include<stdio.h>
extern int main_ret;
extern int test_count;
extern int test_pass;

#define FORMAT_INT 0
#define FORMAT_STR 2

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_STR(expect, actual) EXPECT_EQ_BASE(test_strcmp((expect),(actual)), expect, actual, "%s")

int test_strcmp(const char* str1, const char* str2);

#endif // !__TESTFRAMEWORK_H
