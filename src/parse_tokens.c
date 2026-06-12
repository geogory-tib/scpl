#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/error.h"
#include "include/parse_tokens.h"
#include "include/gtd.h"
static int tok_precedence_table[] = {
  -3,
  0,
  0,
  1,
  1,
  2,
  2,
  -3,
  -3,
  -2,
  -3,
};
static struct
{
  token_slice input;
  size_t current_pos;
  program_t prog;
}parser;

token_t pull_tok()
{
  if(parser.current_pos >= parser.input.len)
	return  (token_t){.type = TOK_EOF};
  parser.current_pos++;
  return parser.input.buffer[parser.current_pos - 1];
}
token_t peek_tok()
{
  if(parser.current_pos >= parser.input.len)
	return  (token_t){.type = TOK_EOF};
  return parser.input.buffer[parser.current_pos];
}
// for when there is an error
void eat_line()
{
  for(token_t tok = pull_tok();tok.type != TOK_SCOLON;tok = pull_tok()){
	//just pulls the till the end of the line
  }
}

void parse_func_args(function_t *func){
  // TODO -- handle args;
  for(token_t tok = pull_tok();tok.type == TOK_CPAREN;tok = pull_tok()){
	
  }
  pull_tok();
}
int check_type(token_t tok)
{
  for(int i = 0;i < parser.prog.types.len;i++){
	if(tok.len > strlen(parser.prog.types.buffer[i].name))
	  continue;
	if(!strncmp(tok.raw,parser.prog.types.buffer[i].name, tok.len)){
	  return i;
	}
  }
  return -1;
}
int find_local_var(token_t tok,function_t func)
{
    for(int i = 0;i < func.var_table.len;i++){
	if(tok.len > func.var_table.buffer[i].name.len || tok.len > func.var_table.buffer[i].name.len)
	  continue;
	if(!strncmp(tok.raw,func.var_table.buffer[i].name.raw, tok.len)){
	  return i;
	}
  }
  return -1;
}
// eats remaining tokens until a semicolon is the next tok;
void prat_parse_eat_line()
{
  for(token_t tok = peek_tok();tok.type != TOK_SCOLON;tok = peek_tok()){
	  pull_tok();
  }
}
int get_precedence(token_t tok)
{
  return tok_precedence_table[tok.type];
}

void create_ir(function_t *func, token_t tok)
{
  switch(tok.type){
  case TOK_PLUS:
	{
	  ir_t plus_ir = {.type = OP_ADD};
	  dyn_appendM(func->instructions, plus_ir);
	  break;
	}
  case TOK_MINUS:
	{
	  ir_t minus_ir = {.type = OP_SUB};
	  dyn_appendM(func->instructions, minus_ir);
	  break;
	}
  case TOK_STAR:
	{
	  ir_t mult_ir = {.type = OP_MULT};
	  dyn_appendM(func->instructions,mult_ir);
	  break;
	}
  case TOK_NUM:
	{
	  ir_t num_ir = {.type = OP_LOADIM,.arg = tok.val};
	  dyn_appendM(func->instructions, num_ir);
	  break;
	}
  default:
	break;
  case TOK_SYMBOL:
	{
	  int var_index = find_local_var(tok, *func);
	  if(var_index == -1){
		prat_parse_eat_line();
		throw_error_tok("Undeclared variable", tok);
		break;
	  }
	  ir_t load_var = {.type = OP_LOAD,.arg = var_index};
	  dyn_appendM(func->instructions, load_var);
	  break;
	}
	
  }
}

void pratt_parse(function_t *func,int weight)
{
  token_t left = {0};
  token_t op;
  left = pull_tok();
  create_ir(func, left);
  while(get_precedence(peek_tok()) > weight){
	op = pull_tok();
	pratt_parse(func, get_precedence(op));
	create_ir(func, op);
  }
  if(peek_tok().type == TOK_SCOLON && weight == 0){
	pull_tok();
	return;
  }
}

void parse_var_assignment(function_t *func)
{
  pratt_parse(func, 0);
}
void parse_var_dec(function_t *func)
{
   token_t new_var_name = pull_tok();
   if(new_var_name.type != TOK_SYMBOL){
	 throw_error_tok("Expected symbol after var keyword", new_var_name);
	 eat_line();
	 return;
   }
   token_t type_name = pull_tok();
   if(type_name.type != TOK_SYMBOL){
	 throw_error_tok("Expected symbol after var name ", type_name);
	 eat_line();
	 return;
   }
   int type_ind = check_type(type_name);
   if(type_ind == -1){
	 throw_error_tok("Expected valid after var name ", type_name);
   }
   var_t new_var = {.name = new_var_name,.type_index = type_ind};
   if(func->var_table.len > 0){
	 size_t prev_var_type = func->var_table.buffer[func->var_table.len - 1].type_index;
	 size_t prev_var_size = parser.prog.types.buffer[prev_var_type].size;
	 new_var.offset = func->var_table.buffer[func->var_table.len - 1].offset + prev_var_size;
   }
   dyn_appendM(func->var_table,new_var);
   if(peek_tok().type == TOK_EQUAL){
	 pull_tok();
	 parse_var_assignment(func);
	 ir_t inst = {.type = OP_STORE, .arg = func->var_table.len - 1};
	 dyn_appendM(func->instructions,inst);
   }else if(peek_tok().type == TOK_SCOLON){
	 ir_t load = {.type = OP_LOADIM, .arg = 0};
	 dyn_appendM(func->instructions, load);
	 ir_t store = {.type = OP_LOAD, .arg = func->var_table.len -1};
	 dyn_appendM(func->instructions, store);
   }
}
 void parse_return(function_t *func)
{
  pratt_parse(func, 0);
  ir_t ret_ir = {.type = OP_RET};
  dyn_appendM(func->instructions, ret_ir);
}
void parse_function_body(function_t *func)
{
  for(token_t tok = pull_tok();;tok = pull_tok()){
	switch(tok.type){
	case TOK_KEYWORD:
	  switch(tok.val){
	  case VAR_IND:
		{
		  parse_var_dec(func);
		  break;
		}
	  case RETURN_IND:
		{
		  parse_return(func);
		  break;
		}
	  case  END_IND:
		{
		  goto end;
		}
	  default:
		panic("UNIMPLEMENTED FEATURE");
	  }
	}
  }
 end:
}
void parse_function()
{
  token_t function_name = pull_tok();
  if(function_name.type != TOK_SYMBOL){
	throw_error_tok("Invaild token type for function", function_name);
	eat_line();
	return;
  }
  function_t func = {.name = function_name};
  parse_func_args(&func);
  token_t ret_type = pull_tok();
  int type_index = check_type(ret_type);
  if(type_index == -1){
	throw_error_tok("Unknown Type", ret_type);
	eat_line();
  }
  token_t line_end = peek_tok();
  if(line_end.type != TOK_SCOLON){
	throw_error_tok("Expected Semi-Colon before token ", line_end);
  }
  pull_tok();
  if(peek_tok().type == TOK_KEYWORD && peek_tok().val == BEGIN_IND){
	pull_tok();
	parse_function_body(&func);
  }
  dyn_appendM(parser.prog.functions, func);
}

void parse_into_ir()
{
  for(token_t tok = pull_tok();tok.type != TOK_EOF;tok = pull_tok()){
	switch(tok.type){
	case TOK_KEYWORD:
	  {
		if(tok.val != FUNCTION_IND){
		  throw_error_tok("expected function token", tok);
		}
		parse_function();
		break;
	  }
	default:
	  {
		throw_error_tok("Unsupported tok", tok);
	  }
	}
  }
}

extern char *create_asm(program_t prog);
program_t parse_tokens(token_slice in)
{
  memset(&parser, 0, sizeof(parser));
  parser.input = in;
  type_t word = {.name = "word",sizeof(void*)};
  dyn_appendM(parser.prog.types, word);
  parse_into_ir();
  return parser.prog;
}
