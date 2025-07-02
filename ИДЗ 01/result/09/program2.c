#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 128

// Flags: if fifo was created in this program it should be disposed in that program.
int disposeFifo1 = 0;
int disposeFifo2 = 0;

/// @brief Сhecks the correctness of the input arguments.
/// @param argc Count of arguments.
/// @param argv Array of arguments.
/// @return Returns -1 if something went wrong.
int checkArguments(int argc, char** argv);

/// @brief Process the data.
/// @param input_fd Descriptor to read the content from.
/// @param output_fd Descriptor to write the result to.
/// @return Returns -1 if something went wrong.
int processData(int input_fd, int output_fd);

/// @brief Checks symbol to be allowed.
/// @param ch Symbol to check.
/// @return Returns 0 if symbol is allowed, another wise returns -1.
int isHexCharacter(char ch);

/// @brief Creates fifo it is not created yet.
/// @param fifo Fifo to create.
/// @return Returns -1 if creation failed. \
/// @return Returns 0 if fifo was already created. \
/// @return Returns 1 if fifo is just created.
int createFifo(char* fifo);

/// @brief Creates 2 fifo.
/// @param fifo1 First fifo.
/// @param fifo2 Second fifo.
/// @param fd Descriptors of 2 fifo.
/// @return Returns -1 if something went wrong otherwise returns 0;
int initialize(char* fifo1, char* fifo2, int fd[2]);

/// @brief Disposes fifos if nessesary.
/// @param fifo1 First fifo.
/// @param fifo2 Second fifo.
void dispose(char* fifo1, char* fifo2, int fd[2]);

int main(int argc, char** argv) {
  // Check arguments.
  if (checkArguments(argc, argv) == -1) return 0;

  // descroptors of fifo1 and fifo2.
  int fd[2];

  // Create pipes.
  if (initialize(argv[1], argv[2], fd) == -1) return 0;

  printf("Initialization was successful\n");

  // Process data.
  printf("Start processing the data\n");
  if (processData(fd[0], fd[1]) == -1) {
    printf("Processing failed\n");
    dispose(argv[1], argv[2], fd);
    return 0;
  }

  printf("The program completed successfully\n");
  dispose(argv[1], argv[2], fd);
  return 0;
}

int checkArguments(int argc, char** argv) {
  // Check amount of arguments.
  if (argc != 3) {
    printf("Incorrect usage:\n%s <fifo1> <fifo2>\n", argv[0]);
    return -1;
  }

  return 0;
}

int processData(int input_fd, int output_fd) {
  char readBuf[BUF_SIZE], writeBuf[BUF_SIZE];
  int readSize, writeSize = 0;
  int isHex = -1, groupCount = 0;

  readSize = read(input_fd, readBuf, BUF_SIZE);
  write(output_fd, "Digits: ", 8);

  while (0 < readSize) {
    // Process read data.
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

    // if there is something to write.
    if (writeSize) {
      int size = write(output_fd, writeBuf, writeSize);
      if (size != writeSize) printf("Part of processed data is not transfered to writing process\n");
      writeSize = 0;
    }

    readSize = read(input_fd, readBuf, BUF_SIZE);
  }

  // Write total count of groups.
  writeSize = sprintf(writeBuf, "\nCount of groups: %d", groupCount);

  // Write data to the descriptor.
  int size = write(output_fd, writeBuf, writeSize);
  if (size != writeSize) {
    printf("Part of processed data is not transfered to writing process\n");
  }

  return 0;
}

int isHexCharacter(char ch) {
  if ('0' <= ch && ch <= '9') return 0;
  if ('A' <= ch && ch <= 'F') return 0;
  if ('a' <= ch && ch <= 'f') return 0;
  return -1;
}

/// @brief Creates fifo it is not created yet.
/// @param fifo Fifo to create.
/// @return Returns -1 if creation failed. \
/// @return Returns 0 if fifo was already created. \
/// @return Returns 1 if fifo is just created.
int createFifo(char* fifo) {
  if (access(fifo, F_OK) != -1) return 0;    // Check if fifo is already created.
  if (mkfifo(fifo, 0666) != -1) return 1;    // Create fifo.
  return access(fifo, F_OK) == -1 ? -1 : 0;  // Last check if it is created.
}

/// @brief Creates 2 fifo.
/// @param fifo1 First fifo.
/// @param fifo2 Second fifo.
/// @param fd Descriptors of 2 fifo.
/// @return Returns -1 if something went wrong otherwise returns 0;
int initialize(char* fifo1, char* fifo2, int fd[2]) {
  // Create fifo1.
  disposeFifo1 = createFifo(fifo1);
  if (disposeFifo1 == -1) {
    printf("Can not create fifo %s\n", fifo1);
    return -1;
  }

  // Create fifo2.
  disposeFifo2 = createFifo(fifo2);
  if (disposeFifo2 == -1) {
    printf("Can not create fifo %s\n", fifo2);
    unlink(fifo1);
    return -1;
  }

  // Open fifo1.
  fd[0] = open(fifo1, O_RDONLY);
  if (fd[0] == -1) {
    printf("Can not open fifo %s\n", fifo1);
    unlink(fifo1);
    unlink(fifo2);
    return -1;
  }

  // Open fifo2.
  fd[1] = open(fifo2, O_WRONLY);
  if (fd[1] == -1) {
    printf("Can not open fifo %s\n", fifo2);
    close(fd[0]);
    unlink(fifo1);
    unlink(fifo2);
    return -1;
  }

  return 0;
}

/// @brief Disposes fifos if nessesary.
/// @param fifo1 First fifo.
/// @param fifo2 Second fifo.
void dispose(char* fifo1, char* fifo2, int fd[2]) {
  // Close fifo and then unlink it.
  close(fd[0]);
  close(fd[1]);
  if (disposeFifo1) unlink(fifo1);
  if (disposeFifo2) unlink(fifo2);
}
