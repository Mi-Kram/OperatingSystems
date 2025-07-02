// file-syscall.c
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int len = 14;
// int len = 5;
// int len = 25;
// int len = 13;

int main() {
  int     fd;
  size_t  size;
  const char    string[] = "Hello, world!";
  // int len = sizeof(string);

  if((fd = open("my-out-file-file", O_WRONLY | O_CREAT, 0666)) < 0) {
    printf("Can\'t open file\n");
    exit(-1);
  }

  size = write(fd, string, len);
  printf("size = %ld\n", size);

  if(size != len) {
    printf("Can\'t write all string\n");
    exit(-1);
  }

  if(close(fd) < 0) {
    printf("Can\'t close file\n");
  }

  return 0;
}
