#ifndef GENERAL_H
#define GENERAL_H

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

#define SEM_IVANOV 0
#define SEM_PETROV 1
#define SEM_NECHEPORUK 2
#define SEM_COUNT 3

struct msgbuf {
  long mtype;     
  char mtext[64];
};

key_t get_ipc_key() {
  return ftok(".", 0);
}


#endif
