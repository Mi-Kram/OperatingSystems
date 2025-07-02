#ifndef GENERAL_H
#define GENERAL_H

#define MAX_MSG_LEN 128

#define SERVER_ID 1
#define IVANOV_ID 2
#define IVANOV_DONE_ID 3
#define PETROV_ID 4
#define NECHEPORUK_ID 5
#define LOGGER_ID 6
#define EXIT_ID 7

typedef struct {
  int from_id;
  char message[MAX_MSG_LEN];
} Message;

#endif
