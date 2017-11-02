#include "Scanner.h"
#include "../Catalog.h"
#include "../BaseStruct/dict.h"
#include <stdio.h>

static Dict *dict;//¹Ø¼ü×Ö±í
static DictType dtype = { NULL,NULL,strcmp,NULL,NULL,dict_str_hashfunction };
static int char_type(char);
static Token* token_new(char* value, int cnum, int lnum, int tokentype);

int char_type(char ch) {
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

Token* token_new(char* value, int cnum, int lnum, int tokentype) {
	Token* token_;
	token_ = mem_alloc(sizeof(Token));
	token_->c_num = cnum;
	token_->l_num = lnum;
	token_->token_type = tokentype;
	LIST_INIT(&token_->list_head);
	token_->value_ = value;
	return token_;
}

static Token* get_next_token(char* errmsg,char** ptrsqlstr,int* c_num,int* l_num) {
	char** src_ = ptrsqlstr;
	char* p = *src_;
	//char* checkp = p;
	char ch;
	while (ch = *p) {
		if (ch == ' ' || ch == ';') {
			(*src_)++;
			p++;
			(*c_num)++;

		}else if (ch == '\n') {
			(*l_num)++;
			p++;
			(*src_)++;
		}else {
			size_t len_ = 0;
			int chartype;
			switch (chartype = char_type(ch))
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
				char* value = NULL;
				int token_type = INT;
				int num_ = 0;
				while (ch >= '0' && ch <= '9') {
					num_ = num_ * 10 + (ch - '0');
					(*c_num)++;
					ch = *++*src_;
				}
				if (ch == '.') {
					(*c_num)++;
					ch = *++*src_;
					token_type = FLOAT;
					int carry = 1;

					while (ch >= '0' && ch <= '9') {
						num_ = num_ * 10 + (ch - '0');
						(*c_num)++;
						ch = *++*src_;
						carry *= 10;
					}
					float f = (num_ * 1.0) / carry;

					memcpy(&value, &f, sizeof(void*));
				}
				else memcpy(&value, &num_, sizeof(void*));

				return token_new(value, *c_num, *l_num, token_type);
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
				int token_type = 0;
				switch (ch) {
				case '(': 
					token_type = LB; 
					break;
				case ')': token_type = RB; break;
				case ',': token_type = COMMA; break;
				case ';': token_type = SEM; break;
				case '.': token_type = DOT; break; 
				case '=': token_type = EQUAL; break; 
				case '*': token_type = STAR; break;
				case '!':
					if (*(*src_)++ != '=') {
						sprintf(errmsg, " %d ÐÐ, %d ÁÐ ´íÎó£º%s\n", *l_num, *c_num, "È±ÉÙ '=' ");
						break;
					}
					token_type = NOT_EQUAL;
				case '>':
					if (*(*src_)++ != '=') //*(*src_++) 
						token_type = GREATERTHAN;
					token_type = GREATER_OR_EQ;
				case '<':
					if (*(*src_)++ != '=')
						token_type = LESSTHAN;
					token_type = LESS_OR_EQ;
				default: 
						sprintf(errmsg, " %d ÐÐ, %d ÁÐ ´íÎó\n", *l_num, *c_num);
					break;
				}

				return token_new(NULL, *c_num, *l_num, token_type);

			}

			char* endchar = p + len_;
			char cpychar = *endchar;
			*endchar = '\0';

			//char *token_str = mem_alloc(len_ + 1);
			//for (size_t i = 0; i < len_; ++i) *token_str++ = *p++;
			//*token_str = '\0';
			//token_str -= len_;
			int token_type;
			Token *token_ = NULL;
			if (token_type = dict_get_value(dict, p)) {
				token_ = token_new(NULL, *c_num, *l_num, token_type);
				//mem_free(token_str);
			}else {
				char *token_str = mem_alloc(len_ + 1);
				memcpy(token_str, p, len_ + 1);
				token_ = token_new(token_str, *c_num, *l_num, chartype);
			}
			*endchar = cpychar;
			return token_;
		}
	}
	return NULL;
}

void token_del(Token* token){
	mem_free(token->value_);
	mem_free(token);
}

Token* scanner(char* errmsg,char** ptrsqlstr, int * c_num, int * l_num){
	Token* token_head;
	if ((token_head = get_next_token(errmsg, ptrsqlstr, c_num, l_num)) == NULL) {
		sprintf(errmsg, "%s", "¿Õsql×Ö·û´®\n");
		return NULL;
	}
	for (Token *token_next = get_next_token(errmsg,ptrsqlstr,c_num,l_num), *head = &token_head->list_head;
		token_next; token_next = get_next_token(errmsg,ptrsqlstr,c_num,l_num)) 

		LIST_ADD_TAIL(&head->list_head, &token_next->list_head);
	return token_head;
}



void move_value(void** src, void** dest){
	*dest = *src;
	*src = NULL;
}

//void token_print(Token * t){
//	printf()
//}

void init_key_word(void) { 

	dict = new_dict_len(&dtype,35);

	dict_add_entry(dict, "database", DATABASE);
	dict_add_entry(dict, "use", USE);
	dict_add_entry(dict, "create", CREATE);
	dict_add_entry(dict, "table", TABLE);
	dict_add_entry(dict, "index", INDEX);
	dict_add_entry(dict, "char", CHAR);
	dict_add_entry(dict, "int", INT);
	dict_add_entry(dict, "float", FLOAT);
	dict_add_entry(dict, "primary", PRIMARY);
	dict_add_entry(dict, "key", KEY);
	dict_add_entry(dict, "update", UPDATE);
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
	dict_add_entry(dict, "unique", UNIQUE);
	dict_add_entry(dict, "NOT", NOT);
	dict_add_entry(dict, "NULL", NULL_);
	dict_add_entry(dict, "insert", INSERT);
	dict_add_entry(dict, "into", INTO);
	dict_add_entry(dict, "values", VALUES);
	dict_add_entry(dict, "on", ON);
	dict_add_entry(dict, "set", SET);
	dict_add_entry(dict, "as", AS);
	dict_add_entry(dict, "delete", DELETE);
	dict_add_entry(dict, "union", UNION);
	dict_add_entry(dict, "avg", AVG);
	dict_add_entry(dict, "max", MAX);
	dict_add_entry(dict, "min", MIN);
	//dict_add_entry(dict, ">", GREATERTHAN);
	//dict_add_entry(dict, "<", LESSTHAN);
	//dict_add_entry(dict, "=", EQUAL);
}