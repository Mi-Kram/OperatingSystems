// umask.c
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

const int len = 14;

int main() {
  int     fd;
  size_t  size;
  char    string[] = "Hello, world!";

  (void)umask(0);
  // (void)umask(0333);
  // (void)umask(0777);

  if((fd = open("my-out-file", O_WRONLY | O_CREAT, 0666)) < 0){
    printf("Can\'t open file\n");
    exit(-1);
  }

  size = write(fd, string, len);

  if(size != len) {
    printf("Can\'t write all string\n");
    exit(-1);
  }

  if(close(fd) < 0) {
    printf("Can\'t close file\n");
  }

  return 0;
}
