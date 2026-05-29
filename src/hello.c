#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  char* s = (char*)malloc(19);
  int n = 0;
  scanf("%s", s);
  printf("qA  we");
  free(s);
  //char c = getc(stdin);
  //write(1, "qwe", 3);
  //open("utils.c", O_RDONLY, 1);
  return 0;
}
