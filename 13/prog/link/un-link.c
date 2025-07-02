// Программа демонстрирующая удаление ссылки
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    if(argc != 2) {
        printf("Use: %s <path name>\n", argv[0]);
        exit(1);
    }
    if(unlink(argv[1])) {
        printf("Unlink error = %d\n", errno);
    }
    return errno;
}
