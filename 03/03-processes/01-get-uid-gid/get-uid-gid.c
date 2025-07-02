#include <unistd.h>
#include <stdio.h>

int main() {
    uid_t uid = getuid();
    gid_t gid = getgid();
    // printf("My uid = %d, my gid = %d\n", (int)uid, (int)gid);
    printf("My uid = %d, my gid = %d\n", uid, gid);
    return 0;
}
