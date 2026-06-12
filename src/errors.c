#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/error.h"
#include "include/gtd.h"
#include "include/lexer.h"
error_slice error_stack;



void throw_error(char *msg,char *file_name,size_t line,size_t col)
{
  error_t err = {msg,file_name,line,col};
  dyn_appendM(error_stack, err);
}

void throw_error_tok(char *msg,token_t tok)
{
  char *err_msg = calloc(ERROR_MSG_SIZE, sizeof(char));
  sprintf(err_msg, "%s '%.*s'", msg,tok.len,tok.raw);
  error_t err = {err_msg,tok.filename,tok.line,tok.col};
  dyn_appendM(error_stack, err);
}

void print_error(error_t err)
{
  fprintf(stderr,"\nIn File %s @ %d:%d - %s\n",err.file_name,err.line,err.col,err.msg);
}

void print_error_stack()
{
  for(int i = 0;i < error_stack.len;i++){
	print_error(error_stack.buffer[i]);
  }
}





