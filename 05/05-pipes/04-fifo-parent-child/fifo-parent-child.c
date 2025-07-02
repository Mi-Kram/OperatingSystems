// fifo-parent-child.c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
   int     fd, result;
   size_t  size;
   char    resstring[14];
   char msg[] = "Hello, world!";
   int msg_size = sizeof(msg);
   char    name[] = "aaa.fifo";

   (void)umask(0);

   // Вариант с проверкой требует удаления именованного канала
   // if(mknod(name, S_IFIFO | 0666, 0) < 0){
   //   printf("Can\'t create FIFO\n");
   //   exit(-1);
   // }

   // Вариант без проверки позволяет использовать уже созданный FIFO
   mknod(name, S_IFIFO | 0666, 0);

   if((result = fork()) < 0){
      printf("Can\t fork child\n");
      exit(-1);
   } else if (result > 0) { /* Parent process */
      sleep(2);
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
        printf("parent: Can\'t close FIFO\n"); exit(-1);
      }
      printf("Parent exit\n");
   } else { /* Child process */
      if((fd = open(name, O_RDONLY)) < 0){
         printf("Can\'t open FIFO for reading\n");
         exit(-1);
      }
      size = read(fd, resstring, msg_size);
      if(size < 0){
         printf("Can\'t read string from FIFO\n");
         exit(-1);
      }
      printf("Child exit, resstring: %s\n", resstring);
      if(close(fd) < 0){
        printf("child: Can\'t close FIFO\n"); exit(-1);
      }
   }
   return 0;
}
