#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 5000

/// @brief Сhecks the correctness of the input arguments.
/// @param argc Count of arguments.
/// @param argv Array of arguments.
/// @return Returns -1 if something went wrong.
int checkArguments(int argc, char** argv);

/// @brief Sets the pipe file descriptors in arrays.
/// @param inputPipe Pipe for input.
/// @param outputPipe Pipe for output.
/// @return Returns -1 if something went wrong.
int createPipes(int inputPipe[2], int outputPipe[2]);

/// @brief Reads file and write the input to the write_fd descriptor.
/// @param path Path to the file to read.
/// @param pipe Descriptors of a pipe to write the file content.
/// @return Returns -1 if something went wrong.
int readFile(char* path, int pipe[2]);

/// @brief Process the data.
/// @param inputPipe Descriptors of a pipe to read from.
/// @param outputPipe Descriptors of a pipe to write to.
/// @return Returns -1 if something went wrong.
int processData(int inputPipe[2], int outputPipe[2]);

/// @brief Checks symbol to be allowed.
/// @param ch Symbol to check.
/// @return Returns 0 if symbol is allowed, another wise returns -1.
int isHexCharacter(char ch);

/// @brief Write data from to the descriptor to file.
/// @param pipe Descriptors of a pipe to read from.
/// @param path File path to write to.
/// @return Returns -1 if something went wrong.
int writeData(int pipe[2], char* path);

/// @brief Close pipes.
/// @param pipe1 First pipe.
/// @param pipe2 Second pipe.
void closePipes(int pipe1[2], int pipe2[2]);

int main(int argc, char** argv) {
  // Check arguments.
  if (checkArguments(argc, argv) == -1) return 0;

  // Create pipes.
  int inputPipe[2], outputPipe[2];
  if (createPipes(inputPipe, outputPipe) == -1) return 0;

  printf("Initialization was successful\n");
  printf("Starting the reading process from the file %s\n", argv[1]);

  // Read file.
  if (readFile(argv[1], inputPipe) == -1) {
    printf("Reading failed\n");
    closePipes(inputPipe, outputPipe);
    return 0;
  }

  printf("Starting the processing process\n");

  // Process data.
  if (processData(inputPipe, outputPipe) == -1) {
    printf("Processing failed\n");
    closePipes(inputPipe, outputPipe);
    return 0;
  }

  printf("Starting the writing process to the file %s\n", argv[2]);

  // Write data.
  if (writeData(outputPipe, argv[2]) == -1) {
    printf("Writing failed\n");
    closePipes(inputPipe, outputPipe);
    return 0;
  }

  closePipes(inputPipe, outputPipe);

  wait(NULL);
  wait(NULL);
  wait(NULL);
  
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

int createPipes(int inputPipe[2], int outputPipe[2]) {
  // Create pipe for input.
  if (pipe(inputPipe) == -1) {
    printf("Can not create pipe\n");
    return -1;
  }

  // Create pipe for output.
  if (pipe(outputPipe) == -1) {
    close(inputPipe[0]);
    close(inputPipe[1]);
    printf("Can not create pipe\n");
    return -1;
  }

  return 0;
}

int readFile(char* path, int pipe[2]) {
  pid_t childPid = fork();

  // If could not create a process.
  if (childPid == -1) {
    printf("Can not create a process\n");
    return -1;
  }

  // If parent process.
  if (childPid) return 0;

  // If child process then read file.

  close(pipe[0]);

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

  // Store read data to the pipe.
  int writeSize = write(pipe[1], buf, readSize);
  if (writeSize != readSize) {
    printf("Can not write data to a pipe\n");
    exit(-1);
  }

  close(pipe[1]);
  exit(0);
}

int processData(int inputPipe[2], int outputPipe[2]) {
  pid_t childPid = fork();

  // If could not create a process.
  if (childPid == -1) {
    printf("Can not create a process\n");
    return -1;
  }

  // If parent process.
  if (childPid) return 0;

  // If child process then process data.

  close(inputPipe[1]);
  close(outputPipe[0]);

  char readBuf[BUF_SIZE], writeBuf[BUF_SIZE];
  int readSize = read(inputPipe[0], readBuf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the data from pipe\n");
    exit(-1);
  }

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

  // Write data to the descriptor.
  size = write(outputPipe[1], writeBuf, writeSize);
  if (size != writeSize) {
    printf("Can not write the data to the pipe\n");
    exit(-1);
  }

  close(inputPipe[0]);
  close(outputPipe[1]);
  exit(0);
}

int isHexCharacter(char ch) {
  if ('0' <= ch && ch <= '9') return 0;
  if ('A' <= ch && ch <= 'F') return 0;
  if ('a' <= ch && ch <= 'f') return 0;
  return -1;
}

int writeData(int pipe[2], char* path) {
  pid_t childPid = fork();

  // If could not create a process.
  if (childPid == -1) {
    printf("Can not create a process\n");
    return -1;
  }

  // If parent process.
  if (childPid) return 0;

  // If child process then write data.

  close(pipe[1]);

  // Read data.
  int buf[BUF_SIZE];
  int readSize = read(pipe[0], buf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the file\n");
    exit(-1);
  }

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
  close(pipe[0]);
  exit(0);
}

void closePipes(int pipe1[2], int pipe2[2]) {
  close(pipe1[0]);
  close(pipe1[1]);
  close(pipe2[0]);
  close(pipe2[1]);
}


