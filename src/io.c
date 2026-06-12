#include <stdio.h>
#include <stdlib.h>
#include "include/gtd.h"
#include <assert.h>
/*
  Most IO operations will go here for organization sake

  -- Geogory


 */

 typedef struct
{
  char *buf;
  size_t len;
  size_t cap;
}char_slice;


static inline void append_char_slice(char_slice *slice,char *input,int len){
  for(int i = 0; i < len;i++){
	if(slice->len >= slice->cap){
	  slice->buf = realloc(slice->buf,(slice->cap + 1) * 2);
	  slice->cap = (slice->cap + 1) * 2;
	}
	slice->buf[slice->len] = input[i];
	slice->len++;
  }
}

char *read_file(char *file_name, size_t *size)
{
  char_slice slice = {0};
  char buf[1024];
  FILE *file = fopen(file_name, "rb");
  assert(file != NULL);
  while(!feof(file)){
	size_t n =  fread(buf, sizeof(char), sizeof(buf), file);
	append_char_slice(&slice, buf, n);
  }
  *size = slice.len;
  return slice.buf;
}
