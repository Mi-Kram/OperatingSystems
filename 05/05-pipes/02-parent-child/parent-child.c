// parent-child.c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

const int buf_size = 25;
// const int buf_size = 10;
const int mes_size = 14;
// const int mes_size = 5;

int main() {
  int    fd[2], result;
  size_t size;
  char   str_buf[buf_size];
  for(int i = 0; i < buf_size; ++i) {
    str_buf[i] = '@';
  }

  if(pipe(fd) < 0) {
    printf("Can\'t open pipe\n");
    exit(-1);
  }

  result = fork();
  if(result < 0) {
    printf("Can\'t fork child\n");
    exit(-1);
  } else if (result > 0) { /* Parent process */
    sleep(2);
    if(close(fd[0]) < 0){
        printf("parent: Can\'t close reading side of pipe\n"); exit(-1);
    }
    size = write(fd[1], "Hello, world!", mes_size);
    if(size != mes_size){
      printf("Can\'t write all string to pipe\n");
      exit(-1);
    }
    if(close(fd[1]) < 0) {
      printf("parent: Can\'t close writing side of pipe\n");
      exit(-1);
    }
    printf("Parent exit\n");
  } else { /* Child process */
    printf("Child started\n");
    // sleep(1);
    if(close(fd[1]) < 0){
      printf("child: Can\'t close writing side of pipe\n"); exit(-1);
    }
    size = read(fd[0], str_buf, mes_size);
    if(size < 0){
        printf("Can\'t read string from pipe\n");
        exit(-1);
    }
    printf("Child exit, str_buf: %s\n", str_buf);
    if(close(fd[0]) < 0){
        printf("child: Can\'t close reading side of pipe\n"); exit(-1);
    }
  }

  return 0;
}
