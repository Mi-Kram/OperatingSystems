#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 128
#define MSG_TYPE 1

struct message {
  long type;
  char text[BUF_SIZE];
};

/// @brief Сhecks the correctness of the input arguments.
/// @param argc Count of arguments.
/// @param argv Array of arguments.
/// @return Returns -1 if something went wrong.
int checkArguments(int argc, char** argv);

/// @brief Process the data.
/// @param input_msg Message ID to read the content from.
/// @param output_msg Message ID to write the result to.
/// @return Returns -1 if something went wrong.
int processData(int input_msg, int output_msg);

/// @brief Checks symbol to be allowed.
/// @param ch Symbol to check.
/// @return Returns 0 if symbol is allowed, another wise returns -1.
int isHexCharacter(char ch);

/// @brief Creates 2 messages.
/// @param key1 First message key.
/// @param key2 Second message key.
/// @param msg ID of 2 message.
/// @return Returns -1 if something went wrong otherwise returns 0;
int initialize(char* key1, char* key2, int msg[2]);

int main(int argc, char** argv) {
  // Check arguments.
  if (checkArguments(argc, argv) == -1) return 0;

  int msg[2];
  if (initialize(argv[1], argv[2], msg) == -1) return 0;
  printf("Program is successfully initialized\n");

  printf("Start processing information...\n");
  processData(msg[0], msg[1]);

  printf("The program is successfully completed.\n");
  return 0;
}

int checkArguments(int argc, char** argv) {
  // Check amount of arguments.
  if (argc != 3) {
    printf("Incorrect usage:\n%s <key1> <key2>\n", argv[0]);
    return -1;
  }

  // Get message keys.
  int key1 = atoi(argv[1]);
  int key2 = atoi(argv[2]);

  // Check correctness of keys.
  if (!key1 || !key2 || key1 == key2) {
    printf("Incorrect usage:\nKeys should be integers and key1 != key2\n");
    return -1;
  }

  return 0;
}

int processData(int input_msg, int output_msg) {
  struct message read_msg;
  struct message write_msg;
  write_msg.type = MSG_TYPE;

  int readSize, writeSize = 0;
  int isHex = -1, groupCount = 0;

  // The initial value to send.
  writeSize = sprintf(write_msg.text, "Digits: ");
  msgsnd(output_msg, &write_msg, writeSize, 0);
  writeSize = 0;

  // read message queue until there is any information.
  while ((readSize = msgrcv(input_msg, &read_msg, BUF_SIZE, MSG_TYPE, 0)) > 0) {
    // Process read data.
    for (int i = 0; i < readSize; ++i) {
      // If readBuf[i] is not hex character.
      if (isHexCharacter(read_msg.text[i]) == -1) {
        if (isHex != -1) isHex = -1;
        continue;
      }

      // readBuf[i] is a hex character.

      // If previous character was hex character.
      if (isHex == -1) {
        if (groupCount) write_msg.text[writeSize++] = ',';
        ++groupCount;
      }

      isHex = 0;
      write_msg.text[writeSize++] = read_msg.text[i];
    }

    // if there is something to write.
    if (writeSize) {
      msgsnd(output_msg, &write_msg, writeSize, 0);
      writeSize = 0;
    }
  }

  // Write total count of groups.
  writeSize = sprintf(write_msg.text, "\nCount of groups: %d", groupCount);

  // Send last message with information.
  msgsnd(output_msg, &write_msg, writeSize, 0);
  msgsnd(output_msg, &write_msg, 0, 0); // Send signal that processing is completed.
  return 0;
}

int isHexCharacter(char ch) {
  if ('0' <= ch && ch <= '9') return 0;
  if ('A' <= ch && ch <= 'F') return 0;
  if ('a' <= ch && ch <= 'f') return 0;
  return -1;
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
