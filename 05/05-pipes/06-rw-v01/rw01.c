// rw01.c
// Первый читатель-писатель
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
   int     fd01;
   int     fd02;
   size_t  size;
   char msg[] = "Hello from 01!";
   int msg_size = sizeof(msg);
   int buf_size = 20;
   char    str_buf[buf_size];
   char    name01[] = "rw01.fifo";
   char    name02[] = "rw02.fifo";

   //---------------------------------------------------
   (void)umask(0);

   // Вариант с проверкой требует удаления именованного канала
   // if(mknod(name01, S_IFIFO | 0666, 0) < 0){
   //   printf("Can\'t create FIFO\n");
   //   exit(-1);
   // }

   // Вариант без проверки позволяет использовать уже созданный FIFO
   mknod(name01, S_IFIFO | 0666, 0);

   if((fd01 = open(name01, O_WRONLY)) < 0){
      printf("rw01: Can\'t open FIFO %s for writting\n", name01);
      exit(-1);
   }
   size = write(fd01, msg, msg_size);
   if(size != msg_size){
      printf("rw01: Can\'t write all string to FIFO %s\n", name01);
      exit(-1);
   }

   if(close(fd01) < 0){
      printf("rw01: Can\'t close FIFO %s\n", name01);
      exit(-1);
   }

   //---------------------------------------------------
   // Только читает из созданного канала
   if((fd02 = open(name02, O_RDONLY)) < 0){
      printf("rw01: Can\'t open FIFO %s for reading\n", name02);
      exit(-1);
   }
   size = read(fd02, str_buf, buf_size);
   if(size < 0){
      printf("rw01 Can\'t read string from FIFO %s\n", name02);
      exit(-1);
   }
   printf("rw01 exit, str_buf: %s\n", str_buf);
   if(close(fd02) < 0){
      printf("rw01: Can\'t close FIFO %s\n", name02);
      exit(-1);
   }
   printf("rw01 exit\n");
   return 0;
}
