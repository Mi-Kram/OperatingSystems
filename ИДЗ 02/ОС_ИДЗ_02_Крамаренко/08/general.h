#ifndef GENERAL_H
#define GENERAL_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SEM_IVANOV 0
#define SEM_PETROV 1
#define SEM_NECHEPORUK 2
#define SEM_COUNT 3

typedef struct {
  char item[64];
  int value;
  int running;
} SharedData;

key_t get_ipc_key() {
  return ftok(".", 0);
}

#endif
