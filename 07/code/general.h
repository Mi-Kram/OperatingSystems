#ifndef GENERAL_H
#define GENERAL_H

typedef struct {
  int value;
  int stop;
} data_t;

#define SHM_SIZE sizeof(data_t)
#define SHM_KEY 0x1111

#endif
