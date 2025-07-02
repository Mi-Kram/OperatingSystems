// Программа демонстрирующая формирование жесткой ссылки
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("Use: %s <old path> <new path>\n", argv[0]);
        exit(1);
    }
    if(symlink(argv[1], argv[2])) {
        printf("Link error = %d\n", errno);
    }
    return errno;
}
