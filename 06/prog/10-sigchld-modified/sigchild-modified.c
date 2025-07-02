#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <stdlib.h>

int count = 0;

void my_handler(int nsig){
    int status;
    pid_t pid;

    // if((pid = waitpid(-1, &status, 0))  > 0){ // Вместо этого
    // while((pid = waitpid(-1, &status, 0))  > 0){ // Тоже работает?
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){ // Наверное основной
      if ((status & 0xff) == 0) {
           printf("%d) Process %d was exited with status %d\n",
                  count++, pid, status >> 8);
      } else if ((status & 0xff00) == 0){
           printf("Process %d killed by signal %d %s\n", pid, status &0x7f,
           (status & 0x80) ? "with core file" : "without core file");
      }
    }
    if(pid < 0) {
       printf("Some error on waitpid errno = %d\n", errno);
    }
    else if (pid == 0) {
      printf("pid = 0\n");
    }
}

int main(void){
    pid_t pid;
    int i,j;
    (void) signal(SIGCHLD, my_handler);
    // for(i=0; i<50; i++){
    for(i=0; i<10; i++){
      // sleep(1);
       if((pid = fork()) < 0){
          printf("Can\'t fork child 1\n");
          exit(1);
       } else if (pid == 0){
          /* Child  */
          for(j=1; j<10000000; )j++;
          exit(200+i);
       }
    }
    while(1);
    return 0;

}
