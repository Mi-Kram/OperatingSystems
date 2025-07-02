#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pathname> <newlength>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        printf("Can not open file: %s\n", argv[1]);
    }

    int len = atoi(argv[2]);
    if(len <= 0) {
        fprintf(stderr, "Incorrect len = %d\n", len);
        exit(EXIT_FAILURE);
    }

    ftruncate(fd, len);

    if(close(fd) < 0) {
        printf("Can\'t close file\n");
    }

    return 0;
}
