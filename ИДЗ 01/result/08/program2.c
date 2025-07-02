#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 5000

/// @brief Сhecks the correctness of the input arguments.
/// @param argc Count of arguments.
/// @param argv Array of arguments.
/// @return Returns -1 if something went wrong.
int checkArguments(int argc, char** argv);

/// @brief Process the data.
/// @return Returns -1 if something went wrong.
int processData(char* fifo1, char* fifo2);

/// @brief Checks symbol to be allowed.
/// @param ch Symbol to check.
/// @return Returns 0 if symbol is allowed, another wise returns -1.
int isHexCharacter(char ch);

int main(int argc, char** argv) {
  // Check arguments.
  if (checkArguments(argc, argv) == -1) return 0;

  printf("Starting the processing process\n");

  // Process data.
  if (processData(argv[1], argv[2]) == -1) {
    printf("Processing failed\n");
    return 0;
  }
  
  printf("The program completed successfully\n");

  return 0;
}

int checkArguments(int argc, char** argv) {
  // Check amount of arguments.
  if (argc != 3) {
    printf("Incorrect usage:\n%s <fifo1> <fifo2>\n", argv[0]);
    return -1;
  }

  // Check the fifo file.
  int inputFileCode = access(argv[1], F_OK);
  if (inputFileCode) {
    printf("Fifo \"%s\" is not found\n", argv[1]);
    return -1;
  }

  // Check the fifo file.
  inputFileCode = access(argv[2], F_OK);
  if (inputFileCode) {
    printf("Fifo \"%s\" is not found\n", argv[2]);
    return -1;
  }

  return 0;
}

int processData(char* fifo1, char* fifo2) {
  int input_fd = open(fifo1, O_RDONLY);
  if (input_fd == -1) {
    printf("Can not open a named pipe\n");
    return -1;
  }

  char readBuf[BUF_SIZE];
  int readSize = read(input_fd, readBuf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the data from pipe\n");
    return -1;
  }

  close(input_fd);

  char writeBuf[BUF_SIZE];
  int isHex = -1, groupCount = 0;
  int writeSize = sprintf(writeBuf, "Digits: ");

  // Iterating over an array.
  for (int i = 0; i < readSize; ++i) {
    // If readBuf[i] is not hex character.
    if (isHexCharacter(readBuf[i]) == -1) {
      if (isHex != -1) isHex = -1;
      continue;
    }

    // readBuf[i] is hex character.

    // If previous character was hex character.
    if (isHex == -1) {
      if (groupCount) writeBuf[writeSize++] = ',';
      ++groupCount;
    }

    isHex = 0;
    writeBuf[writeSize++] = readBuf[i];
  }

  // Write total count of groups.
  int size = sprintf(writeBuf + writeSize, "\nCount of groups: %d", groupCount);
  writeSize += size;

  int output_fd = open(fifo2, O_WRONLY);
  if (input_fd == -1) {
    printf("Can not open a named pipe\n");
    return -1;
  }

  // Write data to the descriptor.
  size = write(output_fd, writeBuf, writeSize);
  if (size != writeSize) {
    printf("Can not write the data to the pipe\n");
    return -1;
  }

  close(output_fd);
  return 0;
}

int isHexCharacter(char ch) {
  if ('0' <= ch && ch <= '9') return 0;
  if ('A' <= ch && ch <= 'F') return 0;
  if ('a' <= ch && ch <= 'f') return 0;
  return -1;
}


