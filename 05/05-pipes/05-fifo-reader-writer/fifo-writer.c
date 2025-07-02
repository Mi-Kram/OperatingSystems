// fifo-writer.c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
   int     fd, result;
   size_t  size;
   char msg[] = "Hello, reader!";
   int msg_size = sizeof(msg);
   char    name[] = "bbb.fifo";

   (void)umask(0);

   // Вариант с проверкой требует удаления именованного канала
   // if(mknod(name, S_IFIFO | 0666, 0) < 0){
   //   printf("Can\'t create FIFO\n");
   //   exit(-1);
   // }

   // Вариант без проверки позволяет использовать уже созданный FIFO
   mknod(name, S_IFIFO | 0666, 0);

   if((fd = open(name, O_WRONLY)) < 0){
      printf("Can\'t open FIFO for writting\n");
      exit(-1);
   }
   size = write(fd, msg, msg_size);
   if(size != msg_size){
      printf("Can\'t write all string to FIFO\n");
      exit(-1);
   }
   if(close(fd) < 0){
      printf("Writer: Can\'t close FIFO\n"); exit(-1);
   }
   printf("Writer exit\n");
   return 0;
}
