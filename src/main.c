#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/gtd.h"
#include "include/io.h"
#include "include/error.h"
#include "include/parse_tokens.h"
extern char *create_asm(program_t prog);
int main(int argc,char *argv[]){
  if(argc < 3){
	panic("Not enough arguments");
  }
  size_t buf_len;
  char *file_contents = read_file(argv[1], &buf_len);
  token_slice tokens =  lex_tokens(file_contents,buf_len,argv[1]);
  if(error_stack.len > 0){
	print_error_stack();
	return EXIT_FAILURE;
  }
  program_t prog = parse_tokens(tokens);
  if(error_stack.len > 0){
	print_error_stack();
	return EXIT_FAILURE;
  }
  char *asm_out = create_asm(prog);
  FILE *output_file = fopen(argv[2], "w");
  fwrite(asm_out, sizeof(char), strlen(asm_out), output_file);
  fclose(output_file);
  return 0;
}
