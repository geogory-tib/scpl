#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "include/lexer.h"
#include "include/gtd.h"
#include "include/error.h"

static const char keyword_table[][15] = {
  "to",
  "if",
  "end",
  "for",
  "var",
  "begin",
  "return",
  "function"
};

static int check_if_keyword(token_t tok){
  for(int i = 0;i < __keywords_end;i++){
	size_t keyword_len = strlen(keyword_table[i]);
	if(tok.len > keyword_len || tok.len < keyword_len)
	  continue;
	if(!strncmp(tok.raw, keyword_table[i], tok.len)){
	  return i;
	}
  }
  return -1;
}

static struct{
  char *input;
  char *filename;
  token_slice output;
  size_t current_pos;
  size_t input_len;
  size_t current_col;
  size_t current_line;
}lexer;


char pull_ch(){
  if(lexer.current_pos >= lexer.input_len)
	return 0;
  lexer.current_pos++;
  if(lexer.input[lexer.current_pos - 1] == '\n'){
	lexer.current_col = 0;
	lexer.current_line++;
  }else
	lexer.current_col++;
  
  return lexer.input[lexer.current_pos - 1];
}

char peek_ch(){
  if(lexer.current_pos >= lexer.input_len)
	return 0;
  return lexer.input[lexer.current_pos];
}

void parse_symbol()
{
  token_t tok = {.line = lexer.current_line,.col = lexer.current_col};
  tok.raw = &lexer.input[lexer.current_pos];
  for(char ch = peek_ch(); isalnum(ch) || ch == '_';ch = peek_ch()){
	tok.len++;
	pull_ch();
  }
  int index = check_if_keyword(tok);
  if(index == -1){
	tok.type = TOK_SYMBOL;
  }else{
	tok.type = TOK_KEYWORD;
	tok.val = index;
  }
  dyn_appendM(lexer.output,tok);
}
void lex_number()
{
  token_t tok = {.line = lexer.current_line,.col = lexer.current_col};
  tok.raw = &lexer.input[lexer.current_pos];
  for(char ch = peek_ch();isdigit(ch);ch = peek_ch()){
	tok.len++;
	pull_ch();
  }
  char *null_termed_num = calloc(tok.len + 1, sizeof(char));
  strncpy(null_termed_num, tok.raw, tok.len);
  
  tok.val = atoi(null_termed_num);
  tok.type =  TOK_NUM;
  dyn_appendM(lexer.output, tok);
}
void lex_input()
{
  for(char ch = peek_ch(); ch != 0;ch = peek_ch()){
	if(isalpha(ch)){
	  parse_symbol();
	}else if(isalnum(ch)){
	  lex_number();
	}else{
	  switch(ch){
	  case ';':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_SCOLON,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case '(':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_OPAREN,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case ')':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_CPAREN,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case '+':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_PLUS,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case '*':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_STAR,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case '-':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_MINUS,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case '/':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_SLASH,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case '=':
		{
		  token_t tok = {
			.raw = &lexer.input[lexer.current_pos],
			.len = 1,
			.type = TOK_EQUAL,
			.line = lexer.current_line,
			.col = lexer.current_col,
			.filename = lexer.filename
		  };
		  pull_ch();
		  dyn_appendM(lexer.output,tok);
		  break;
		}
	  case '\n':
	  case '\r':
	  case '\t':
	  case ' ':
		{
		  pull_ch();
		  break;
		}
	  case 0:
		token_t tok = {
		  .raw = NULL,
		  .type = TOK_EOF,
		  .filename = lexer.filename
		};
		dyn_appendM(lexer.output, tok);
		goto exit;
	  default:
		{
		  char *msg_buf = calloc(ERROR_MSG_SIZE, sizeof(char));
		  sprintf(msg_buf,"Invaild Char '%c'", ch);
		  throw_error(msg_buf, lexer.filename,lexer.current_line + 1, lexer.current_col);
		  pull_ch();
		}
	  }
	  
		
	}
  }
 exit:
  return;
}

token_slice lex_tokens(char *input, size_t input_len,char *filename)
{
  memset(&lexer, 0, sizeof(lexer));
  lexer.input = input;
  lexer.input_len = input_len;
  lexer.filename = filename;
  lex_input();
  return lexer.output;
}

