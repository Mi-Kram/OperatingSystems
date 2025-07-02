#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
   char pid[255];
   printf("%d: Before\n", getpid());
   fork();
   printf("%d: 1\n", getpid());
   fork();
   printf("%d: 2\n", getpid());
   fork();
   printf("%d: 3\n", getpid());
   sleep(1);
   sprintf(pid, "PID = %d, PPID = %d \n", getpid(), getppid());
   write(STDOUT_FILENO, pid, strlen(pid));
   exit(0);
}
