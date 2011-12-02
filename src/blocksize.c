#include "cma.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MSIZE 1024*17
// unsigned char mem[MSIZE];

int main(int argc, char * argv[]) {
  int size = 1, msize, works = 0;
  if(argc > 1) {
    msize = atoi(argv[1]);
  }else {
    return 1;
  }
  
  void *mem = malloc(msize);
  
  int i = 0;
  char *strings = NULL;
  class_memory(mem,msize); // give it memory allocator
  
  strings = (char *)class_malloc(size); //allocate from classmem library
  
  while(strings != NULL){
    size = size * 2;
     class_free(strings);
    strings = (char *)class_malloc(size); //allocate from classmem library
    if(strings != NULL)
      works = size;
 }
  class_stats();
printf("Maximum block size: %i \n", works);

}
