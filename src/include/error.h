#ifndef ERROR_H
#define ERROR_H
#define ERROR_MSG_SIZE 512
#include "error.h"
#include "lexer.h"
typedef enum{
  LEXER_STAGE = 0,
  TOKEN_PARSING,
  ASM_OUTPUT,
}comp_stage_t;

typedef struct{
  char *msg;
  char *file_name;
  size_t line;
  size_t  col;
  comp_stage_t stage;
}error_t;

typedef struct{
  error_t *buffer;
  size_t len;
  size_t cap;
}error_slice;

extern error_slice error_stack;


void throw_error(char *msg,char *file_name,size_t line,size_t col);
void print_error_stack();
void throw_error_tok(char *msg,token_t tok);
#endif
