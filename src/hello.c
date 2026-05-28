#include <unistd.h>
#include <fcntl.h>

int main() {
  //char c = getc(stdin);
  write(1, "qwe", 3);
  //open("utils.c", O_RDONLY, 1);
  return 0;
}
