// Программа демонстрирующая формирование символической (мягкой) ссылки
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("Use: %s <file name> <new symlink>\n", argv[0]);
        exit(1);
    }
    if(symlink(argv[1], argv[2])) {
        printf("Simlink error = %d\n", errno);
    }
    return errno;
}
