#ifndef PARSE_TOKENS_H
#define PARSE_TOKENS_H
#include "lexer.h"
typedef enum {
  OP_LOAD = 0,
  OP_LOADIM,
  OP_STORE,
  OP_STOREIM,
  OP_ADD,
  OP_SUB,
  OP_MULT,
  OP_DIV,
  OP_RET,
  OP_PUSH_ARG,
  OP_GET_ADDR,
  OP_DEREF_ADDR,
  OP_DEREF_ASSIGN,
  OP_CLEAR_STACK,
  OP_CMP_EQUAL,
  OP_CMP_NOT_EQ,
  OP_CALL,
  OP_JMPT,
  OP_JMPF,
  LABEL
}ir_type;
// dont know if I need the scope enum
typedef enum{
  SCOPE_FUNC = 0,
  SCOPE_IF,
  SCOPE_FOR
}scope_type;
typedef enum{
  VAR_BINARY = 0,
  VAR_POINTER,
  VAR_ARRAY
}var_type;
typedef struct{
  token_t name;
  int type_index;
  size_t offset;
  int v_type;
  char stored; // tell if the var has been stored yet;
}var_t;

typedef struct{
  var_t *buffer;
  size_t len;
  size_t cap; 
}var_slice;

typedef struct
{
  var_slice args;
  int func_ind;
}ir_func_call;
typedef struct
{
  ir_type type;
  union{
	int arg;
	ir_func_call fargs;
  }args;
}ir_t;

typedef struct{
  ir_t *buffer;
  size_t len;
  size_t cap;
}ir_slice;


typedef struct{
  char *name;
  int size;
  var_type type;
}type_t;

typedef struct{
  type_t *buffer;
  size_t len;
  size_t cap; 
}type_slice;



typedef struct{
  token_t name;
  ir_slice instructions;
  var_slice var_table;
  var_slice args;
  type_t ret_type;
}function_t;

typedef struct{
  function_t *buffer;
  size_t len;
  size_t cap;
}func_slice;

typedef struct{
  func_slice functions;
  type_slice types;
  var_slice globals;
}program_t;
program_t parse_tokens(token_slice in);


#endif
