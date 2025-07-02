#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid, ppid, chpid;
    chpid = fork();
    pid  = getpid();
    ppid = getppid();

    if(chpid <= -1) {
        printf("Incorrect fork syscall\n");
    } else if (chpid == 0) {
        printf("I am child. ");
        printf("My pid = %d, my parent is %d and I have no child\n",
               (int)pid, (int)ppid);
    } else {
        printf("I am parent. ");
        printf("My pid = %d, my parent is %d, my child is %d\n",
               (int)pid, (int)ppid, (int)chpid);
    }
    return 0;
}
