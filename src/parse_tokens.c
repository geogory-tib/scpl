#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/error.h"
#include "include/parse_tokens.h"
#include "include/gtd.h"
//pointer size of arch
extern const size_t WORD_SIZE;
static int tok_precedence_table[] = {
  -3, // KEYWORD
  0, // NUM
  0, // SYMBOL
  1, // PLUS 
  1, // MINUS
  2, // STAR
  2, // SLASH
  -3, // OPAREN
  -3, // OPAREN
  -2, // SCOLON
  -3, // EQUAL
  4,  // @
  4, // ^
  -3 // EOF
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
//  TODO -- implement arrays
int get_v_type(){
  if(peek_tok().type == TOK_CARROT){
	pull_tok();
	return VAR_POINTER;
  }else if(peek_tok().type == TOK_SYMBOL){
	return VAR_BINARY;
  }
}

int check_type(token_t tok)
{
  for(int i = 0;i < parser.prog.types.len;i++){
	if(tok.len > strlen(parser.prog.types.buffer[i].name) ||  tok.len < strlen(parser.prog.types.buffer[i].name))
	  continue;
	if(!strncmp(tok.raw,parser.prog.types.buffer[i].name, tok.len)){
	  return i;
	}
  }
  return -1;
}
// must check type before providing type 
var_t create_new_stack_var(function_t *func,token_t name,int type_ind,var_type v_type)
{
  var_t var = {0};
  var.name = name;
  var.type_index = type_ind;
  int var_ind = func->var_table.len - 1;
  int type_size = parser.prog.types.buffer[type_ind].size;
  if(func->var_table.len == 0){
	if(v_type == VAR_POINTER){
	  var.offset = WORD_SIZE;
	}else{
	  var.offset = type_size;
	}
  }
  else{
	var_t prev_var = func->var_table.buffer[var_ind];
	if(v_type == VAR_POINTER){
	  var.offset = prev_var.offset + WORD_SIZE;
	}else {
	  var.offset = prev_var.offset + type_size;
	}
  }
  var.v_type = v_type;
  return var;
}
void parse_func_args(function_t *func){

  if(pull_tok().type != TOK_OPAREN){
	eat_line();
	throw_error_tok("Expected paren after function name",func->name);
	return;
  }
  for(;;){
	var_t arg_var;
	token_t name = pull_tok();
	if(name.type == TOK_CPAREN || name.type == TOK_EOF){
	  break;
	}
	if(name.type != TOK_SYMBOL){
	  throw_error_tok("Expected symbol for function argument name", func->name);
	}
	var_type v_type = get_v_type();
	token_t type_name = pull_tok();
	if(type_name.type != TOK_SYMBOL){
	  throw_error_tok("Expected type after function argument",type_name);
	}
	int type_index = check_type(type_name);
	if(type_index == -1){
	  throw_error_tok("Undefined Type", type_name);
	}
	token_t semi_colon = pull_tok();
	if(semi_colon.type != TOK_SCOLON){
	  throw_error_tok("Expected ';' after argument type", type_name);
	}
	arg_var = create_new_stack_var(func, name, type_index,v_type);
	dyn_appendM(func->args, arg_var);
	dyn_appendM(func->var_table, arg_var);
  }
}

int check_if_func_defined(token_t tok)
{
   for(int i = 0;i < parser.prog.types.len;i++){
	if(tok.len > parser.prog.functions.buffer[i].name.len ||  tok.len < parser.prog.functions.buffer[i].name.len)
	  continue;
	if(!strncmp(tok.raw,parser.prog.functions.buffer[i].name.raw, tok.len)){
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
void parse_call_args(function_t *func,int func_index,token_t call_name);
void handle_function_call(token_t symbol,function_t *func)
{
  
  ir_t call_ir = {.type = OP_CALL};
  //TODO -- parse_call_args(); remove the bottom if clause after implementing this.

  int func_index = check_if_func_defined(symbol);
  if(func_index == -1){
	prat_parse_eat_line();
	throw_error_tok("Undefined Function", symbol);
	return;
  }
  parse_call_args(func,func_index,symbol);
  call_ir.args.fargs.func_ind = func_index;
  dyn_appendM(func->instructions, call_ir);
}
int get_precedence(token_t tok)
{
  return tok_precedence_table[tok.type];
}
void pratt_parse(function_t *func,int weight,var_type type,int paren);
void create_ir(function_t *func, token_t tok,var_type type,int paren)
{
  switch(tok.type){
  case TOK_PLUS:
	{
	  if(type != VAR_POINTER && type != VAR_BINARY){
		throw_error_tok("Invaild variable type for variable", tok);
		prat_parse_eat_line();
	  }
	  ir_t plus_ir = {.type = OP_ADD};
	  dyn_appendM(func->instructions, plus_ir);
	  break;
	}
  case TOK_MINUS:
	{
	  if(type != VAR_POINTER && type != VAR_BINARY){
		throw_error_tok("Invaild variable type for variable", tok);
		prat_parse_eat_line();
	  }
	  ir_t minus_ir = {.type = OP_SUB};
	  dyn_appendM(func->instructions, minus_ir);
	  break;
	}
  case TOK_SLASH:
	{
	  if(type != VAR_POINTER && type != VAR_BINARY){
		throw_error_tok("Invaild variable type for variable", tok);
		prat_parse_eat_line();
	  }
	  ir_t div_ir  = {.type = OP_DIV};
	  dyn_appendM(func->instructions,div_ir);
	  break;
	}
  case TOK_STAR:
	{
	  if(type != VAR_POINTER && type != VAR_BINARY){
		throw_error_tok("Invaild variable type for variable", tok);
		prat_parse_eat_line();
	  }
	  ir_t mult_ir = {.type = OP_MULT};
	  dyn_appendM(func->instructions,mult_ir);
	  break;
	}
  case TOK_NUM:
	{
	  if(type != VAR_POINTER && type != VAR_BINARY){
		throw_error_tok("Invaild variable type for variable", tok);
		prat_parse_eat_line();
	  }
	  ir_t num_ir = {.type = OP_LOADIM,.args.arg = tok.val};
	  dyn_appendM(func->instructions, num_ir);
	  break;
	}
  case TOK_OPAREN:
	{
	  pratt_parse(func, 0,type,paren + 1);
	  break;
	}
  case TOK_AT:
	{
	  token_t var_name = pull_tok();
	  if(var_name.type != TOK_SYMBOL){
		throw_error_tok("Expected symbol for variable name", var_name);
		prat_parse_eat_line();
	  }
	  int var_ind = find_local_var(var_name,*func);
	  if(var_ind == -1){
		throw_error_tok("Udefined variable or function", var_name);
		prat_parse_eat_line();
	  }
	  ir_t pointer_ir = {.type = OP_GET_ADDR,.args.arg = var_ind};
	  dyn_appendM(func->instructions, pointer_ir);
	  break;
	}
  case TOK_CARROT:
	{
	  token_t var_name = pull_tok();
	  if(var_name.type != TOK_SYMBOL){
		throw_error_tok("Expected symbol for variable name", var_name);
		prat_parse_eat_line();
	  }
	  int var_ind = find_local_var(var_name,*func);
	  if(var_ind == -1){
		throw_error_tok("Udefined variable or function", var_name);
		prat_parse_eat_line();
		return;
	  }
	  if(func->var_table.buffer[var_ind].v_type != VAR_POINTER){
		throw_error_tok("Expected pointer variable", var_name);
		prat_parse_eat_line();
		return;
	  }
	  ir_t deref_op = {.type = OP_DEREF_ADDR,.args.arg = var_ind};
	  dyn_appendM(func->instructions, deref_op);
	  break;
	}
  case TOK_SYMBOL:
	{
	  if(peek_tok().type ==  TOK_OPAREN){
		pull_tok();
		handle_function_call(tok, func);
	  }else{
		int var_index = find_local_var(tok, *func);
		if(var_index == -1){
		  prat_parse_eat_line();
		  throw_error_tok("Undeclared variable", tok);
		  break;
		}
		ir_t load_var = {.type = OP_LOAD,.args.arg = var_index};
		dyn_appendM(func->instructions, load_var);
		break;
	  }
	}
  default:
	{
	  prat_parse_eat_line();
	  throw_error_tok("Invaild token in expression", tok);
	  break;
  	}
  }
}

void pratt_parse(function_t *func,int weight,var_type type,int paren)
{
  token_t left = {0};
  token_t op;
  left = pull_tok();
  create_ir(func, left,type,paren);
  while(get_precedence(peek_tok()) > weight){
	op = pull_tok();
	pratt_parse(func, get_precedence(op),type,paren);
	create_ir(func, op,type,paren);
  }
  if(peek_tok().type == TOK_CPAREN && paren > 0 && weight == 0){
	pull_tok();
	return;
  }else if(peek_tok().type == TOK_CPAREN && paren == 0){
	throw_error_tok("Unexpected closing paren", peek_tok());
	prat_parse_eat_line();
	return;
  }
  if(peek_tok().type == TOK_SCOLON && weight == 0){
	if(paren > 0){
	  throw_error_tok("Expected closing paren", peek_tok());
	}
	pull_tok();
	return;
  }
}
void parse_call_args(function_t *func,int func_called,token_t call_name)
{
  int args_supplied = 0;
  function_t called_func = parser.prog.functions.buffer[func_called];
  for(;;){
	if(peek_tok().type == TOK_CPAREN){
	  pull_tok();
	  break;
	}
	if(called_func.args.len <= args_supplied)
	  break;
	pratt_parse(func, 0,called_func.args.buffer[args_supplied].v_type,0);
	ir_t push_arg = {.type = OP_PUSH_ARG};
	dyn_appendM(func->instructions, push_arg);
	args_supplied++;
  }
  if(args_supplied > called_func.args.len){
	prat_parse_eat_line();
	throw_error_tok("too many args provided for function",call_name);
  }
  if(args_supplied < called_func.args.len){
	prat_parse_eat_line();
	throw_error_tok("too few args provided for function",call_name);
  }
}
//deref assignment
void parse_ptr_assignment(function_t *func,int var_ind)
{
  pratt_parse(func, 0,func->var_table.buffer[var_ind].v_type,0);
  ir_t inst = {.type = OP_DEREF_ASSIGN, .args.arg = var_ind};
  dyn_appendM(func->instructions,inst);
}

void parse_var_assignment(function_t *func,int var_ind)
{
  pratt_parse(func, 0,func->var_table.buffer[var_ind].v_type,0);
  ir_t inst = {.type = OP_STORE, .args.arg = var_ind};
  dyn_appendM(func->instructions,inst);
}


void parse_var_dec(function_t *func)
{
   token_t new_var_name = pull_tok();
   if(new_var_name.type != TOK_SYMBOL){
	 throw_error_tok("Expected symbol after var keyword", new_var_name);
	 eat_line();
	 return;
   }
   var_type v_type = get_v_type();
   token_t type_name = pull_tok();
   if(type_name.type != TOK_SYMBOL){
	 throw_error_tok("Expected symbol after var name ", type_name);
	 eat_line();
	 return;
   }
   int type_ind = check_type(type_name);
   if(type_ind == -1){
	 throw_error_tok("Expected valid type after var name ", type_name);
	 eat_line();
	 return;
   }
   var_t new_var = create_new_stack_var(func, new_var_name, type_ind,v_type);
   dyn_appendM(func->var_table,new_var);
   if(peek_tok().type == TOK_EQUAL){
	 pull_tok();
	 parse_var_assignment(func, func->var_table.len - 1);
   }else if(peek_tok().type == TOK_SCOLON){
	 ir_t load = {.type = OP_LOADIM, .args.arg = 0};
	 dyn_appendM(func->instructions, load);
	 ir_t store = {.type = OP_LOAD, .args.arg = func->var_table.len -1};
	 dyn_appendM(func->instructions, store);
   }
}
 void parse_return(function_t *func)
{
  if(peek_tok().type == TOK_SCOLON){
	if(func->ret_type.size > 0){
	  throw_error_tok("Expected expression before colon", peek_tok());
	  eat_line();
	  return;
	}
  }else{
	pratt_parse(func, 0,func->ret_type.type,0);
  }
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
	  break;
	  case TOK_SYMBOL:
	  {
		token_t next_tok = peek_tok();
		if(next_tok.type == TOK_OPAREN){
		  pull_tok();
		  handle_function_call(tok, func);
		  ir_t clear_stack_ir = {.type = OP_CLEAR_STACK};
		  dyn_appendM(func->instructions, clear_stack_ir);
		}else if(next_tok.type == TOK_EQUAL){
		  pull_tok();
		  int var_index = find_local_var(tok, *func);
		  if(var_index == -1){
			eat_line();
			throw_error_tok("Undefined Variable", tok);
			break;
		  }
		  parse_var_assignment(func, var_index);
		}
		break;
	  }
	case TOK_CARROT:
	  {
		token_t var_name = pull_tok();
		int var_index = find_local_var(var_name, *func);
		if(var_index == -1){
		  eat_line();
		  throw_error_tok("Undefined Variable", tok);
		  break;
		}else if(func->var_table.buffer[var_index].v_type != VAR_POINTER){
		  throw_error_tok("Expected pointer value after '^'", var_name);
		  eat_line();
		  break;
		}
		if(pull_tok().type != TOK_EQUAL){
		  throw_error_tok("Expected  '=' before expression", var_name);
		  eat_line();
		  break;
		}
		parse_ptr_assignment(func,var_index);
		break; 
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
  dyn_appendM(parser.prog.functions, func);
  if(peek_tok().type == TOK_KEYWORD && peek_tok().val == BEGIN_IND){
	pull_tok();
	parse_function_body(&parser.prog.functions.buffer[parser.prog.functions.len - 1]);
  }
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

// entry point for back end
extern char *create_asm(program_t prog);

program_t parse_tokens(token_slice in)
{
  memset(&parser, 0, sizeof(parser));
  parser.input = in;
  type_t word = {.name = "word",sizeof(void*),VAR_BINARY};
  type_t byte = {.name = "byte",.size = 1,VAR_BINARY};
  type_t void_t = {.name = "void",.size = 0,VAR_BINARY};
  dyn_appendM(parser.prog.types, byte);
  dyn_appendM(parser.prog.types, word);
  dyn_appendM(parser.prog.types, void_t);
  parse_into_ir();
  return parser.prog;
}
