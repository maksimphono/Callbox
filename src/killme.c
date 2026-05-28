#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define M(a, b, c) (a #c b)

void g() {
    int fd[2];

    if (pipe(fd) != -1) {
        close(fd[0]);
        close(fd[1]);
    }
}

void f(int mode, char* str) {
    static char* my_str;
    if (mode == 1) {
        my_str = str;
        puts("Remembered!");
    }else {
        puts(my_str);
    }
}

int main() {
    M(4, 5, '<');
    printf("%d\n", 1);
    //f(1, "Qwertyu");
    //f(0, "");
    return 0;
}
