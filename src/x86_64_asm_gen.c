#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/error.h"
#include "include/parse_tokens.h"
#include "include/gtd.h"
const size_t WORD_SIZE = (sizeof(void*));
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
char registers_str[][5] = {
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

char func_arg_reg[][5] = {
  {"rdi"},
  {"rsi"},
  {"rdx"},
  {"rcx"},
  {"r8"},
  {"r9"}
};

char math_instructs[][5] = {
  {"add"},
  {"sub"},
  {"imul"},
  {"div"}
};

char size_tabel[][6] = {
  {"BYTE"},
  {"WORD"},
  {"DWORD"},
  {""}, // empty strs for padding
  {""},
  {""},
  {""},
  {"QWORD"}
};
char rax_size_table[][5] = {
  {"al"},
  {"ax"},
  {"eax"},
  {""}, // empty strs for padding
  {""},
  {""},
  {""},
  {"rax"}
};

typedef struct{
  char *buffer;
  size_t len;
  size_t cap;
}char_slice;
char_slice asm_out;
static unsigned int current_reg = 0;
static size_t stack_size = 0;


void load_var(var_t var, program_t program)
{
  char sprintf_buf[512];
  type_t var_type = program.types.buffer[var.type_index];
  if(var_type.size == WORD_SIZE || var.v_type == VAR_POINTER)
	sprintf(sprintf_buf, "    mov  %s,[rbp - %d]\n",registers_str[current_reg],var.offset);
  else{
	sprintf(sprintf_buf, "    movzx %s,%s PTR [rbp - %d]\n",registers_str[current_reg],size_tabel[var_type.size - 1],var.offset);
  }
  current_reg++;
  dyn_appendManyM(asm_out, sprintf_buf, strlen(sprintf_buf));
}

void deref_pointer(var_t var,program_t program)
{
  char sprintf_buf[512];
  load_var(var, program);
  type_t var_type = program.types.buffer[var.type_index];
  if(var_type.size == WORD_SIZE)
	sprintf(sprintf_buf, "    mov  %s,[%s]\n",registers_str[current_reg],registers_str[current_reg - 1]);
  else{
	sprintf(sprintf_buf,"    xor %s,%s\n",registers_str[current_reg],registers_str[current_reg]);
	dyn_appendManyM(asm_out, sprintf_buf, strlen(sprintf_buf));
	sprintf(sprintf_buf, "    movzx %s,%s PTR [%s]\n",registers_str[current_reg],size_tabel[var_type.size - 1],registers_str[current_reg - 1]);
  }
  dyn_appendManyM(asm_out, sprintf_buf, strlen(sprintf_buf));
  sprintf(sprintf_buf, "    mov %s,%s\n", registers_str[current_reg - 1],registers_str[current_reg]);
  dyn_appendManyM(asm_out, sprintf_buf, strlen(sprintf_buf));
}

void store_var(var_t *var, program_t program)
{
  type_t var_type = program.types.buffer[var->type_index];
  char  sprintf_buff[512];
  if(!var->stored){
	var->stored = 1;
	if(var->v_type ==  VAR_POINTER){
	  sprintf(sprintf_buff,"    sub rsp,%d\n",WORD_SIZE);
	  dyn_appendManyM(asm_out,sprintf_buff,strlen(sprintf_buff));
	  sprintf(sprintf_buff, "    mov  %s PTR [rbp - %zu],%s\n", size_tabel[WORD_SIZE - 1], var->offset,rax_size_table[WORD_SIZE - 1]);
	  dyn_appendManyM(asm_out,sprintf_buff,strlen(sprintf_buff));
	  stack_size += WORD_SIZE;
	}else{
	  sprintf(sprintf_buff,"    sub rsp,%d\n",var_type.size);
	  dyn_appendManyM(asm_out,sprintf_buff,strlen(sprintf_buff));
	  sprintf(sprintf_buff, "    mov  %s PTR [rbp - %zu],%s\n", size_tabel[var_type.size - 1], var->offset,rax_size_table[var_type.size - 1]);
	  dyn_appendManyM(asm_out,sprintf_buff,strlen(sprintf_buff));
	  stack_size += var_type.size;
	}
  }else{
	sprintf(sprintf_buff, "    mov  %s PTR [rbp - %zu],%s\n", size_tabel[var_type.size - 1], var->offset,rax_size_table[var_type.size - 1]);
	 dyn_appendManyM(asm_out,sprintf_buff,strlen(sprintf_buff));
  }
  current_reg = 0;
}
void store_deref_pointer(var_t *var, program_t program)
{
  type_t var_type = program.types.buffer[var->type_index];
  char  sprintf_buff[512];
  load_var(*var, program);
  sprintf(sprintf_buff, "    mov  %s PTR [rbx],%s\n", size_tabel[var_type.size - 1],rax_size_table[var_type.size - 1]);
  dyn_appendManyM(asm_out,sprintf_buff,strlen(sprintf_buff));
  current_reg = 0;
}
void gen_asm(ir_t ir,function_t func,program_t program)
{
  char snprintf_buf[512];
  switch (ir.type){
  case OP_LOADIM:
	{
	  sprintf(snprintf_buf, "    mov %s,%d\n",registers_str[current_reg], ir.args.arg);
	  current_reg++;
	  dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  break;
	}
  case OP_DIV:
  case OP_MULT:
  case OP_SUB:
  case OP_ADD:
	{
	  //	  sprintf(snprintf_buf, "pop", ...)
	  /* char *first_arg_pop = "    pop rbx\n"; */
	  /* dyn_appendManyM(asm_out, first_arg_pop, (strlen(first_arg_pop))); */
	  /* char *second_arg_pop = "    pop rax\n"; */
	  /* dyn_appendManyM(asm_out, second_arg_pop, strlen(second_arg_pop)); */
	  sprintf(snprintf_buf, "    %s %s,%s\n", math_instructs[ir.type - 4],registers_str[current_reg - 2],registers_str[current_reg - 1]);
	  current_reg--;
	  dyn_appendManyM(asm_out,snprintf_buf, strlen(snprintf_buf));
	  /* char *push_result = "    push rax\n"; */
	  /* dyn_appendManyM(asm_out, push_result, strlen(push_result)); */
	  break; 
	}
  case OP_STORE:
	{
	  store_var(&func.var_table.buffer[ir.args.arg], program);
	  break;
	}
  case OP_LOAD:
  {
	load_var(func.var_table.buffer[ir.args.arg], program);
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
  case OP_CALL:
	{
	  // on linux the stack has to be 16 byte algined when calling c functions or anyother langauge for that matter
	  size_t reg_saved_stack_space = 0; //space used to save register contents
	  size_t safe_stack_offset = 0;

	
	  /* for(int i = 0; i < current_reg;i++){ */
	  /* 	sprintf(snprintf_buf, "    push %s\n", registers_str[i]); */
	  /* 	dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf)); */
	  /* } */
	  for(int i = (program.functions.buffer[ir.args.fargs.func_ind].args.len - 1); i >= 0; i--){
		sprintf(snprintf_buf, "    pop %s\n", func_arg_reg[i]);
		dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  }
	  if((stack_size + reg_saved_stack_space) % 16 != 0){
		size_t call_safe_stack_size =  (((reg_saved_stack_space + stack_size) + 15) / 16) * 16;
	    safe_stack_offset = call_safe_stack_size - (stack_size + reg_saved_stack_space);
		sprintf(snprintf_buf, "    sub rsp,%ld\n", safe_stack_offset);
	   	dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  }
	  sprintf(snprintf_buf, "    call %.*s\n",program.functions.buffer[ir.args.fargs.func_ind].name.len,program.functions.buffer[ir.args.fargs.func_ind].name.raw);
	  dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  sprintf(snprintf_buf, "    add rsp,%ld\n",safe_stack_offset);
	  dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  sprintf(snprintf_buf, "    mov %s,rax\n", registers_str[current_reg]);
	  dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  for(int i = current_reg - 1;i >= 0;i--){
		sprintf(snprintf_buf, "    pop %s\n", registers_str[i]);
		dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  }
	  current_reg++;
	  break;
	}
  case OP_PUSH_ARG:
	{
	  // will just push rax after evaulating expr
	  char push_ins[] = "    push rax\n";
	  dyn_appendManyM(asm_out, push_ins,strlen(push_ins));
	  current_reg = 0;
	  break;
	}
  case OP_GET_ADDR:
	{
	  var_t var = func.var_table.buffer[ir.args.arg];
	  sprintf(snprintf_buf, "    lea  %s,[rbp - %d]\n",registers_str[current_reg],var.offset);
	  dyn_appendManyM(asm_out, snprintf_buf, strlen(snprintf_buf));
	  current_reg++;
	  break;
	}
  case OP_DEREF_ADDR:
	{
	  var_t var = func.var_table.buffer[ir.args.arg];
	  deref_pointer(var, program);
	  break;
	}
  case OP_DEREF_ASSIGN:
	{
	  var_t var = func.var_table.buffer[ir.args.arg];
	  store_deref_pointer(&var,program);
	  break;
	}
	case OP_CLEAR_STACK:
	  current_reg = 0;
	  break;
  default:
	panic("UNIMPLEMENTED OPCODE\n");
  }
}

void parse_func(function_t func,program_t program)
{
  dyn_appendManyM(asm_out, func.name.raw, func.name.len);
  char *line_ending = ":\n";
  dyn_appendManyM(asm_out, line_ending,2);
  dyn_appendManyM(asm_out,intel_linux_func_pro, strlen(intel_linux_func_pro));
  char sprintf_buf[64];
  for(int i = 0; i < func.args.len;i++){
	sprintf(sprintf_buf, "    mov rax,%s\n", func_arg_reg[i]);
	dyn_appendManyM(asm_out, sprintf_buf, strlen(sprintf_buf));
	store_var(&func.var_table.buffer[i], program);
  }
  for(size_t i = 0;i < func.instructions.len;i++){
	gen_asm(func.instructions.buffer[i],func,program);
  }
}
void parse_funcs(program_t prog)
{
  for(size_t i = 0; i < prog.functions.len;i++){
	parse_func(prog.functions.buffer[i],prog);
	current_reg = 0;
	stack_size = 0;
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


