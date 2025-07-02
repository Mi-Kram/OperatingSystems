#ifndef GENERAL_H
#define GENERAL_H

#define MAX_MSG_LEN 128

#define SERVER_ID 1
#define IVANOV_ID 2
#define PETROV_ID 3
#define NECHEPORUK_ID 4

#define COMMON_TYPE 0
#define REGISTER_TYPE 1
#define EXIT_TYPE 2
#define DONE_TYPE 3

#pragma pack(push, 1)
typedef struct {
  int from_id;
  int type_id;
  char message[MAX_MSG_LEN];
} Message;
#pragma pack(pop)

#endif
