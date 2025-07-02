#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
   char pid[255];
   fork();
   fork();
   fork();
   sprintf(pid, "PID = %d, PPID = %d \n", getpid(), getppid());
   write(STDOUT_FILENO, pid, strlen(pid));
   exit(0);
}
