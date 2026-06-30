#include <stdio.h>

int main(){
  int val;
  puts("Type a number");
  scanf("%d",&val);
  if(val){
    puts("Hello world!\n");
  }else{
    puts(" Bye World!\n");
  }
  return 0;
}
