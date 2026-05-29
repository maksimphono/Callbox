#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  char* s = (char*)malloc(19);
  int n = 0;
  int c = fork();
  if (c == 0) {
    printf("Child process: %d\n", getpid());

    while(1){
      sleep(10);
    }

    exit(0);
  }
  //n = scanf("%s", s);

  printf("qA  wee");
  fflush(stdout);
  sleep(10000);
  free(s);
  //char c = getc(stdin);
  //write(1, "qwe", 3);
  //open("utils.c", O_RDONLY, 1);
  return 0;
}
