#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 5000

#define FIFO1 "IDZ_01_FIFO_1"
#define FIFO2 "IDZ_01_FIFO_2"

/// @brief Сhecks the correctness of the input arguments.
/// @param argc Count of arguments.
/// @param argv Array of arguments.
/// @return Returns -1 if something went wrong.
int checkArguments(int argc, char** argv);

/// @brief Initialized named pipes.
/// @return Returns -1 if something went wrong.
int createFifos();

/// @brief Reads file and write the input to the FIFO1.
/// @param path Path to the file to read.
/// @return Returns -1 if something went wrong.
int readFile(char* path);

/// @brief Process the data.
/// @return Returns -1 if something went wrong.
int processData();

/// @brief Checks symbol to be allowed.
/// @param ch Symbol to check.
/// @return Returns 0 if symbol is allowed, another wise returns -1.
int isHexCharacter(char ch);

/// @brief Write data to file.
/// @param path File path to write to.
/// @return Returns -1 if something went wrong.
int writeData(char* path);

int main(int argc, char** argv) {
  // Check arguments.
  if (checkArguments(argc, argv) == -1) return 0;

  // Create pipes.
  if (createFifos() == -1) return 0;

  printf("Initialization was successful\n");
  printf("Starting the reading process from the file %s\n", argv[1]);

  // Read file.
  if (readFile(argv[1]) == -1) {
    printf("Reading failed\n");
    unlink(FIFO1);
    unlink(FIFO2);
    return 0;
  }

  printf("Starting the processing process\n");

  // Process data.
  if (processData() == -1) {
    printf("Processing failed\n");
    unlink(FIFO1);
    unlink(FIFO2);
    return 0;
  }

  printf("Starting the writing process to the file %s\n", argv[2]);

  // Write data.
  if (writeData(argv[2]) == -1) {
    printf("Writing failed\n");
    unlink(FIFO1);
    unlink(FIFO2);
    return 0;
  }

  wait(NULL);
  wait(NULL);
  wait(NULL);

  unlink(FIFO1);
  unlink(FIFO2);

  printf("The program completed successfully\n");
  return 0;
}

int checkArguments(int argc, char** argv) {
  // Check amount of arguments.
  if (argc != 3) {
    printf("Incorrect usage:\n%s <input file path> <result file path>\n", argv[0]);
    return -1;
  }

  // Check the input file.
  int inputFileCode = access(argv[1], F_OK);
  if (inputFileCode) {
    printf("File \"%s\" is not found\n", argv[1]);
    return -1;
  }

  return 0;
}

int createFifos() {
  // Create named pipe for input.
  if (mkfifo(FIFO1, 0666)) {
    printf("Can not create a named pipe\n");
    return -1;
  }

  // Create named pipe for output.
  if (mkfifo(FIFO2, 0666)) {
    unlink(FIFO1);
    printf("Can not create a named pipe\n");
    return -1;
  }

  return 0;
}

int readFile(char* path) {
  pid_t childPid = fork();

  // If could not create a process.
  if (childPid == -1) {
    printf("Can not create a process\n");
    return -1;
  }

  // If parent process.
  if (childPid) return 0;

  // If child process then read file.

  // Open file.
  int input_fd = open(path, O_RDONLY);
  if (input_fd == -1) {
    printf("Can not open the file\n");
    exit(-1);
  }

  // Read file.
  int buf[BUF_SIZE];
  int readSize = read(input_fd, buf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the file\n");
    exit(-1);
  }

  close(input_fd);

  // If file contains no content.
  if (!readSize) {
    buf[0] = '\0';
    readSize = 1;
  }

  int output_fd = open(FIFO1, O_WRONLY);
  if (output_fd == -1) {
    printf("Can not open a named pipe\n");
    exit(-1);
  }

  // Store read data to the pipe.
  int writeSize = write(output_fd, buf, readSize);
  if (writeSize != readSize) {
    printf("Can not write data to a named pipe\n");
    exit(-1);
  }

  close(output_fd);
  exit(0);
}

int processData() {
  pid_t childPid = fork();

  // If could not create a process.
  if (childPid == -1) {
    printf("Can not create a process\n");
    return -1;
  }

  // If parent process.
  if (childPid) return 0;

  // If child process then process data.

  int input_fd = open(FIFO1, O_RDONLY);
  if (input_fd == -1) {
    printf("Can not open a named pipe\n");
    exit(-1);
  }

  char readBuf[BUF_SIZE];
  int readSize = read(input_fd, readBuf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the data from a named pipe\n");
    exit(-1);
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

  int output_fd = open(FIFO2, O_WRONLY);
  if (input_fd == -1) {
    printf("Can not open a named pipe\n");
    exit(-1);
  }

  // Write data to the descriptor.
  size = write(output_fd, writeBuf, writeSize);
  if (size != writeSize) {
    printf("Can not write the data to a named pipe\n");
    exit(-1);
  }

  close(output_fd);
  exit(0);
}

int isHexCharacter(char ch) {
  if ('0' <= ch && ch <= '9') return 0;
  if ('A' <= ch && ch <= 'F') return 0;
  if ('a' <= ch && ch <= 'f') return 0;
  return -1;
}

int writeData(char* path) {
  pid_t childPid = fork();

  // If could not create a process.
  if (childPid == -1) {
    printf("Can not create a process\n");
    return -1;
  }

  // If parent process.
  if (childPid) return 0;

  // If child process then write data.

  int input_fd = open(FIFO2, O_RDONLY);
  if (input_fd == -1) {
    printf("Can not open a named pipe\n");
    exit(-1);
  }

  // Read data.
  int buf[BUF_SIZE];
  int readSize = read(input_fd, buf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the file\n");
    exit(-1);
  }

  close(input_fd);

  // Open file.
  int output_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (output_fd == -1) {
    printf("Can not open result file\n");
    exit(-1);
  }

  // Write data.
  int writeSize = write(output_fd, buf, readSize);
  if (writeSize != readSize) {
    printf("Can not write to the file\n");
    exit(-1);
  }

  close(output_fd);
  exit(0);
}
