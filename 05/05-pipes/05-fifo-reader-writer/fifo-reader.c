// fifo-reader.c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
   int     fd;
   size_t  size;
   int buf_size = 20;
   char    str_buf[buf_size];
   char    name[] = "bbb.fifo";

   // Только читает из созданного канала
   if((fd = open(name, O_RDONLY)) < 0){
      printf("Can\'t open FIFO for reading\n");
      exit(-1);
   }
   size = read(fd, str_buf, buf_size);
   if(size < 0){
      printf("Can\'t read string from FIFO\n");
      exit(-1);
   }
   printf("Reader exit, str_buf: %s\n", str_buf);
   if(close(fd) < 0){
      printf("Reader: Can\'t close FIFO\n"); exit(-1);
   }
   return 0;
}
