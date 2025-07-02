// loop-file-read.c
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

const int size = 32;

// Циклическое чтение с использованием системного вызова read
// Например, когда буфер небольшой
int main() {
  int     fd;
  char    buffer[size+1];
  ssize_t read_bytes;

  if((fd = open("loop-file-read.c", O_RDONLY)) < 0){
    printf("Can\'t open file\n");
    exit(-1);
  }

  do {
    read_bytes = read(fd, buffer, size);
    if(read_bytes == -1) {
      printf("Can\'t write this file\n");
      exit(-1);
    }
    buffer[read_bytes] = '\0';
    printf("%s", buffer);
    // write(1, buffer, read_bytes);
  } while(read_bytes == size);

  printf("\nLast read_bytes = %ld\n", read_bytes);

  if(close(fd) < 0) {
    printf("Can\'t close file\n");
  }

  return 0;
}
