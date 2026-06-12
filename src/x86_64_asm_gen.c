#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/error.h"
#include "include/parse_tokens.h"
#include "include/gtd.h"
char *intel_linux_preamble = ".intel_syntax noprefix\n\
.global _start\n\
.text\n\
_start:\n\
    call main\n\
    mov rdi,rax\n\
    mov rax,60\n\
    syscall\n";

  char *intel_linux_func_pro = "    push rbp\n\
    mov rbp,rsp\n";
char *registers_str[][5] = {
  {"rax"},
  {"rbx"},
  {"rcx"},
  {"rdx"},
  {"rsi"},
  {"rdi"},
  {"rdp"},
};
enum register_index{
  rax_pos = 0,
  rbx_pos,
  rcx_pos,
  rdx_pos,
  rsi_pos,
  rdi_pos,
  rdp_pos,
  __reg_index_end__
};

char math_instructs[][5] = {
  {"add"},
  {"sub"},
  {"imul"},
  {"div"}
};


typedef struct{
  char *buffer;
  size_t len;
  size_t cap;
}char_slice;
char_slice asm_out;
static unsigned int current_reg = 0;

void gen_asm(ir_t ir,function_t func)
{
  char snprintf_buf[512];
  switch (ir.type){
  case OP_LOADIM:
	{
	  sprintf(snprintf_buf, "    push %d\n", ir.arg);
	  dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  break;
	}
  case OP_DIV:
  case OP_MULT:
  case OP_SUB:
  case OP_ADD:
	{
	  //	  sprintf(snprintf_buf, "pop", ...)
	  char *first_arg_pop = "    pop rbx\n";
	  dyn_appendManyM(asm_out, first_arg_pop, (strlen(first_arg_pop)));
	  char *second_arg_pop = "    pop rax\n";
	  dyn_appendManyM(asm_out, second_arg_pop, strlen(second_arg_pop));
	  sprintf(snprintf_buf, "    %s rax,rbx\n", math_instructs[ir.type - 4]);
	  dyn_appendManyM(asm_out,snprintf_buf, strlen(snprintf_buf));
	  char *push_result = "    push rax\n";
	  dyn_appendManyM(asm_out, push_result, strlen(push_result));
	  break;
	}
  case OP_STORE:
	{
	  // I know this is stupid, but I am just making this as straight foreward as possible to just get somecode gen out of this thing
	  char *pop_rax = "    pop rax\n";
	  dyn_appendManyM(asm_out,pop_rax, strlen(pop_rax));
	  char *push_rax = "    push rax\n";
	  dyn_appendManyM(asm_out,push_rax,strlen(push_rax));
	  break;
	}
  case OP_LOAD:
  {
	sprintf(snprintf_buf, "    mov rax,[rbp - %d]\n", (func.var_table.buffer[ir.arg].offset + 8));
	dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	break;
  }
  case OP_RET:
	{
	  char leave[] = "    leave\n";
	  dyn_appendManyM(asm_out, leave, (sizeof(leave) - 1));
	  char ret[] = "    ret\n";
	  dyn_appendManyM(asm_out, ret, (sizeof(ret) - 1));
	  break;
	}
  default:
	panic("UNIMPLEMENTED OPCODE\n");
  }
}
void parse_func(function_t func)
{
  dyn_appendManyM(asm_out, func.name.raw, func.name.len);
  char *line_ending = ":\n";
  dyn_appendManyM(asm_out, line_ending,2);
  dyn_appendManyM(asm_out,intel_linux_func_pro, strlen(intel_linux_func_pro));
  for(size_t i = 0;i < func.instructions.len;i++){
	gen_asm(func.instructions.buffer[i],func);
  }
}
void parse_funcs(program_t prog)
{
  for(size_t i = 0; i < prog.functions.len;i++){
	parse_func(prog.functions.buffer[i]);
  }
}
char *create_asm(program_t prog)
{
  memset(&asm_out, 0, sizeof(asm_out));
  dyn_appendManyM(asm_out, intel_linux_preamble,strlen(intel_linux_preamble));
  parse_funcs(prog);
  dyn_appendM(asm_out, '\0');
  return asm_out.buffer;
}


