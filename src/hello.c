#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>

int main() {
  int fd = open("raw_file.txt", O_WRONLY | O_CREAT, 0764);
  close(fd);
  //char c = getc(stdin);
  //write(1, "qwe", 3);
  //open("utils.c", O_RDONLY, 1);
  return 0;
}
