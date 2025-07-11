#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
    int   semid;
    char pathname[]=".";
    key_t key;
    struct sembuf mybuf;

    key = ftok(pathname, 0);

    if((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0){
      printf("Can\'t create semaphore set\n");
      exit(-1);
    }

    mybuf.sem_num = 0;
    mybuf.sem_op  = -1;
    mybuf.sem_flg = 0;

    if(semop(semid, &mybuf, 1) < 0){
      printf("Can\'t add 1 to semaphore\n");
      exit(-1);
    }

    int sem_value = semctl(semid, 0, GETVAL, 0);
    printf("The semaphore value = %d\n", sem_value);
    return 0;
}
