#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid, ppid, chpid;
    int a = 0;

    chpid = fork();

    a = a+1;

    pid  = getpid();
    ppid = getppid();

    printf("My pid = %d, my ppid = %d, child pid = %d, result = %d\n",
           pid, ppid, chpid, a
    );
    printf("addr = %p\n", &a);

    return 0;

}

