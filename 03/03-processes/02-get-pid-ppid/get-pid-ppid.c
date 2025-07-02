#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    // printf("My pid = %d, my ppid = %d\n", (int)pid, (int)ppid);
    printf("My pid = %d, my ppid = %d\n", pid, ppid);
    return 0;
}
