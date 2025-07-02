#ifndef GENERAL_H
#define GENERAL_H

#include <semaphore.h>

typedef struct {
  int value;
  int stop;
} data_t;

#define SHM_NAME "/kramarenko_shm"

#endif
