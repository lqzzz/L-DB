#include "Scanner.h"
#include "Vector.h"
#include "Pair.h"
#include "Catalog.h"
#include "SourceStream.h"
#include "dict.h"
#include <stdio.h>
#define SCANNER_ERROR(buf) do{\
fprintf(stderr, " %d 行, %d 列 错误：%s\n", *l_num, *c_num, buf); goto ERROR;\
}while(0)

static Dict *dict;//关键字表

extern int32_t char_type(char);
extern Token* scanner(Srcstream*);
extern Token* token_new(const char*, uint16_t, uint16_t, int);

static int32_t char_type(char ch) {
	if (ch >= 'a'&&ch <= 'z' ||
		ch >= 'A'&&ch <= 'Z' ||
		ch == '_')
		return ID;
	else if (ch >= '0'&&ch <= '9')
		return NUMBER;
	else if (ch == '\'')
		return TEXT;
	else 
		return SYMBOL;
}

static Token* token_new(const char* value, uint16_t cnum, uint16_t lnum, int tokentype) {
	Token* token_;
	token_ = mem_alloc(sizeof(Token));
	token_->c_num = cnum;
	token_->l_num = lnum;
	token_->token_type = tokentype;
	LIST_INIT(token_);
	token_->value_ = value;
	return token_;
}

static Token* scanner(Srcstream* stream) {
	const char** src_ = &stream->str;
	int *c_num = &stream->c_num;
	int *l_num = &stream->l_num;
	char *p = *src_;
	char ch = *p;
	size_t len_ = 0;
	uint16_t type;
	switch (type = char_type(ch))
	{
	case ID:
		while (ch >= 'a' && ch <= 'z' ||
			ch >= 'A' && ch <= 'Z' ||
			ch >= '0' && ch <= '9' ||
			ch == '_') {
			(*c_num)++;
			len_++;
			ch = *++*src_;
		}
		break;
	case NUMBER: {
		int token_type = INT;
		int num_ = 0;
		while (ch >= '0' && ch <= '9') {
			num_ = num_ * 10 + (ch - '0');
			(*c_num)++;
			len_++;
			ch = *++*src_;
		}
		if (ch == '.') {
			token_type = REAL;
			int carry = 10;
			while (ch >= '0' && ch <= '9') {
				num_ += (ch - '0') / carry;
				(*c_num)++;
				len_++;
				ch = *++*src_;
				carry *= 10;
			}
		}
		Token* token_ = token_new(NULL, *c_num, *l_num, token_type);
		token_->value_ = mem_alloc(sizeof(int));
		memcpy(token_->value_, &num_, sizeof(int));
		return token_;
	}
	case TEXT:
		p = ++*src_;
		(*c_num)++;
		ch = *p;
		while (ch != '\'') {
			(*c_num)++;
			len_++;
			ch = *++*src_;
		}
		++*src_;
		break;
	default:
		++*src_;
		switch (ch) {
		case '(': return token_new(NULL, *c_num, *l_num, LB);
		case ')': return token_new(NULL, *c_num, *l_num, RB);
		case ',': return token_new(NULL, *c_num, *l_num, COMMA);
		case ';': return token_new(NULL, *c_num, *l_num, SEM);
		case '.': return token_new(NULL, *c_num, *l_num, DOT);
		case '=': return token_new(NULL, *c_num, *l_num, EQUAL);
		case '*': return token_new(NULL, *c_num, *l_num, STAR);
		case '!':
			if (*(*src_)++ != '=') SCANNER_ERROR("缺少 '=' ");
			return token_new(NULL, *c_num, *l_num, NOT_EQUAL);
		case '>': 
			if (*(*src_)++ != '=') //*(*src_++) 
				return token_new(NULL, *c_num, *l_num, GREATERTHAN);
			return token_new(NULL, *c_num, *l_num, GREATER_OR_EQ);
		case '<': 
			if (*(*src_)++ != '=')
				return token_new(NULL, *c_num, *l_num, LESSTHAN);
			return token_new(NULL, *c_num, *l_num, LESS_OR_EQ);
		default : break;
		}
		break;
	}
	char *token_str = mem_alloc(len_ + 1);
	for (size_t i = 0; i < len_; ++i) *token_str++ = *p++;
	*token_str = '\0';
	token_str -= len_;
	int token_type;
	Token *token_ = NULL;
	if (token_type = dict_get_value(dict, token_str)) {
		token_ = token_new(NULL, *c_num, *l_num, token_type);
		mem_free(token_str);
	}
	else
		token_ = token_new(token_str, *c_num, *l_num, type);
	return token_;
ERROR:
	return NULL;
}

void token_del(Token* token){
	mem_free(token->value_);
	mem_free(token);
}


Token* get_next_token(Srcstream* stream) {
	const char** src_ = &stream->str;
	int *c_num = &stream->c_num;
	int *l_num = &stream->l_num;
	char ch;
	while (ch = **src_)
		if (ch == ' ' || ch == ';') {
			(*src_)++;
			(*c_num)++;
		}
		else if (ch == '\n') {
			(*l_num)++;
			(*src_)++;
		}
		else return scanner(stream);
		return NULL;
}

void move_value(void** src, void** dest){
	*dest = *src;
	*src = NULL;
}

void init_key_word() { 
	DictType* dict_type = mem_alloc(sizeof(DictType));
	dict_type->key_match = strcmp;
	dict_type->key_dup = NULL;
	dict_type->value_dup = NULL;
	dict = dict_create(dict_type);

	dict_add_entry(dict, "database", DATABASE);
	dict_add_entry(dict, "use", USE);

	dict_add_entry(dict, "create", CREATE);
	dict_add_entry(dict, "table", TABLE);
	dict_add_entry(dict, "index", INDEX);

	dict_add_entry(dict, "text", TEXT);
	dict_add_entry(dict, "real", REAL);
	dict_add_entry(dict, "char", CHAR);
	dict_add_entry(dict, "varchar", VARCHAR);
	dict_add_entry(dict, "time", TIME);
	dict_add_entry(dict, "int", INT);

	dict_add_entry(dict, "primary", PRIMARY);
	dict_add_entry(dict, "key", KEY);
	dict_add_entry(dict, "update", UPDATE);
	dict_add_entry(dict, "foreign", FOREIGN);
	dict_add_entry(dict, "references", REFERENCES);
	dict_add_entry(dict, "null", NULL_);
	dict_add_entry(dict, "not", NOT);

	dict_add_entry(dict, "select", SELECT);
	dict_add_entry(dict, "where", WHERE);
	dict_add_entry(dict, "or", OR);
	dict_add_entry(dict, "and", AND);
	dict_add_entry(dict, "like", LIKE);
	dict_add_entry(dict, "from", FROM);
	dict_add_entry(dict, "group", GROUP);
	dict_add_entry(dict, "by", BY);
	dict_add_entry(dict, "order", ORDER);
	dict_add_entry(dict, "having", HAVING);
	dict_add_entry(dict, "distinct", DISTINCT);

	dict_add_entry(dict, "procedure", PROCEDURE);
	dict_add_entry(dict, "alter", ALTER);
	dict_add_entry(dict, "add", ADD);
	dict_add_entry(dict, "drop", DROP);
	dict_add_entry(dict, "commit", COMMIT);
	dict_add_entry(dict, "transaction", TRANSACTION);
	dict_add_entry(dict, "unique", UNIQUE);

	dict_add_entry(dict, "insert", INSERT);
	dict_add_entry(dict, "into", INTO);
	dict_add_entry(dict, "values", VALUES);
	dict_add_entry(dict, "on", ON);
	dict_add_entry(dict, "set", SET);
	dict_add_entry(dict, "as", AS);
	dict_add_entry(dict, "delete", DELETE);
	dict_add_entry(dict, "union", UNION);
	dict_add_entry(dict, "rollback", ROLLBACK);

	dict_add_entry(dict, "avg", AVG);
	dict_add_entry(dict, "max", MAX);
	dict_add_entry(dict, "min", MIN);

	dict_add_entry(dict, ">", GREATERTHAN);
	dict_add_entry(dict, "<", LESSTHAN);
	dict_add_entry(dict, "=", EQUAL);
}