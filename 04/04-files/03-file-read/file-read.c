// file-read.c
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

const int size = 4096;

// Прямое чтение с использованием системного вызова read
int main() {
    int     fd;
    char    buffer[size];
    // for(int i = 0; i < size; ++i) {
    //   buffer[i] = '@';
    // }
    ssize_t read_bytes;

    if((fd = open("file-read.c", O_RDONLY)) < 0){
      printf("Can\'t open file\n");
      exit(-1);
    }

    read_bytes = read(fd, buffer, size);

    if(read_bytes == -1) {
      printf("Can\'t write this file\n");
      exit(-1);
  }

  buffer[read_bytes] = '\0';
  // printf("%s", buffer);
  write(1, buffer, read_bytes);
  // write(1, buffer, size);

  if(close(fd) < 0) {
    printf("Can\'t close file\n");
  }

  return 0;
}
