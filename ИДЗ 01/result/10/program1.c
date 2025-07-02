#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define BUF_SIZE 128
#define MSG_TYPE 1

struct message {
  long msg_type;
  char msg_text[BUF_SIZE];
};

/// @brief Сhecks the correctness of the input arguments.
/// @param argc Count of arguments.
/// @param argv Array of arguments.
/// @return Returns -1 if something went wrong.
int checkArguments(int argc, char** argv);

/// @brief Creates 2 messages.
/// @param key1 First message key.
/// @param key2 Second message key.
/// @param msg ID of 2 message.
/// @return Returns -1 if something went wrong otherwise returns 0;
int initialize(char* key1, char* key2, int msg[2]);

/// @brief Reads file and put the content to the message queue.
/// @param path Path to file.
/// @param msgid Id of message.
/// @return -1 if something went wrong, otherwise - 0.
int readFile(const char* path, int msgid);

/// @brief Reads the data from the message queue and writes content to the file.
/// @param path Path to file.
/// @param msgid Id of message.
/// @return -1 if something went wrong, otherwise - 0.
int writeFile(const char* path, int msgid);

int main(int argc, char **argv) {
  if (checkArguments(argc, argv) == -1) return 0;

  int msg[2];
  if (initialize(argv[3], argv[4], msg) == -1) return 0;

  printf("Program is successfully initialized\n");
  printf("Start reading file...\n");

  if (readFile(argv[1], msg[0]) == -1) {
    msgctl(msg[0], IPC_RMID, NULL);
    msgctl(msg[1], IPC_RMID, NULL);
    return 0;
  }

  printf("Start writing to the file...\n");
  writeFile(argv[2], msg[1]);

  printf("The program is successfully completed.\n");

  msgctl(msg[0], IPC_RMID, NULL);
  msgctl(msg[1], IPC_RMID, NULL);

  return 0;
}

int checkArguments(int argc, char** argv) {
  // Check amount of arguments.
  if (argc != 5) {
    printf("Incorrect usage:\n%s <input file> <output file> <key1> <key2>\n", argv[0]);
    return -1;
  }

  // Get message keys.
  int key1 = atoi(argv[3]);
  int key2 = atoi(argv[4]);

  // Check correctness of keys.
  if (!key1 || !key2 || key1 == key2) {
    printf("Incorrect usage:\nKeys should be integers and key1 != key2\n");
    return -1;
  }

  return 0;
}

int initialize(char* key1, char* key2, int msg[2]) {
  // Create msg1.
  msg[0] = msgget((key_t)atoi(key1), IPC_CREAT | 0666);
  if (msg[0] == -1) {
    printf("Can not create message with key %s\n", key1);
    return -1;
  }

  // Create msg2.
  msg[1] = msgget((key_t)atoi(key2), IPC_CREAT | 0666);
  if (msg[1] == -1) {
    msgctl(msg[0], IPC_RMID, NULL);
    printf("Can not create message with key %s\n", key2);
    return -1;
  }

  return 0;
}

int readFile(const char* path, int msgid) {
  struct message msg;
  msg.msg_type = MSG_TYPE;
  ssize_t read_size;

  // open file to read.
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    // Send signal that reading is completed. 
    msgsnd(msgid, &msg, 0, 0);
    printf("Cannot open file %s\n", path);
    return -1;
  }

  // until content is read, read it and send the content via message queue.
  while ((read_size = read(fd, msg.msg_text, BUF_SIZE)) > 0) {
    msgsnd(msgid, &msg, read_size, 0);
  }

  // Send signal that reading is completed. 
  msgsnd(msgid, &msg, 0, 0);
  close(fd);
}

int writeFile(const char* path, int msgid) {
  // open file to read.
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    printf("Cannot open file %s\n", path);
    return -1;
  }

  struct message msg;
  int read_size;

  // while message has any data, write the content to the file.
  while ((read_size = msgrcv(msgid, &msg, BUF_SIZE, MSG_TYPE, 0)) > 0) {
    write(fd, msg.msg_text, read_size);
  }

  close(fd);
}