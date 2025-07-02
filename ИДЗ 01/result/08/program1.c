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

/// @brief Sets the pipe file descriptors in arrays.
/// @param fifo1 First fifo.
/// @param fifo2 Second fifo.
/// @return Returns -1 if something went wrong.
int createFifos(char* fifo1, char* fifo2);

/// @brief Reads file and write the input to the write_fd descriptor.
/// @param path Path to the file to read.
/// @param fifo Fifo to write the file content.
/// @return Returns -1 if something went wrong.
int readFile(char* path, char* fifo);

/// @brief Write data to file.
/// @param path File path to write to.
/// @param fifo Fifo to read content from.
/// @return Returns -1 if something went wrong.
int writeData(char* path, char* fifo);

int main(int argc, char** argv) {
  // Check arguments.
  if (checkArguments(argc, argv) == -1) return 0;

  // Create pipes.
  if (createFifos(argv[3], argv[4]) == -1) return 0;

  printf("Initialization was successful\n");

  printf("Starting reading the file %s\n", argv[1]);

  // Read file.
  if (readFile(argv[1], argv[3]) == -1) {
    printf("Reading failed\n");
    unlink(argv[3]);
    unlink(argv[4]);
    return 0;
  }

  printf("Starting writing to the file %s\n", argv[2]);

  // Write data.
  if (writeData(argv[2], argv[4]) == -1) {
    printf("Writing failed\n");
    unlink(argv[3]);
    unlink(argv[4]);
    return 0;
  }
  
  printf("The program completed successfully\n");

  unlink(argv[3]);
  unlink(argv[4]);

  return 0;
}

int checkArguments(int argc, char** argv) {
  // Check amount of arguments.
  if (argc != 5) {
    printf("Incorrect usage:\n%s <input file path> <result file path> <fifo1> <fifo2>\n", argv[0]);
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

int createFifos(char* fifo1, char* fifo2) {
  // Create named pipe for input.
  if (mkfifo(fifo1, 0666)) {
    printf("Can not create a named pipe %s\n", fifo1);
    return -1;
  }

  // Create named pipe for output.
  if (mkfifo(fifo2, 0666)) {
    unlink(fifo1);
    printf("Can not create a named pipe %s\n", fifo2);
    return -1;
  }

  return 0;
}

int readFile(char* path, char* fifo) {
  // Open file.
  int input_fd = open(path, O_RDONLY);
  if (input_fd == -1) {
    printf("Can not open the file\n");
    return -1;
  }

  // Read file.
  int buf[BUF_SIZE];
  int readSize = read(input_fd, buf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the file\n");
    return -1;
  }

  close(input_fd);

  // If file contains no content.
  if (!readSize) {
    buf[0] = '\0';
    readSize = 1;
  }

  int output_fd = open(fifo, O_WRONLY);
  if (output_fd == -1) {
    printf("Can not open a named pipe\n");
    return -1;
  }

  // Store read data to the pipe.
  int writeSize = write(output_fd, buf, readSize);
  if (writeSize != readSize) {
    printf("Can not write data to a pipe\n");
    return -1;
  }

  close(output_fd);
  return 0;
}

int writeData(char* path, char* fifo) {
  int input_fd = open(fifo, O_RDONLY);
  if (input_fd == -1) {
    printf("Can not open a named pipe\n");
    return -1;
  }

  // Read data.
  int buf[BUF_SIZE];
  int readSize = read(input_fd, buf, BUF_SIZE);
  if (readSize == -1) {
    printf("Can not read the pipe\n");
    return -1;
  }
  
  close(input_fd);

  // Open file.
  int output_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (output_fd == -1) {
    printf("Can not open result file\n");
    return -1;
  }

  // Write data.
  int writeSize = write(output_fd, buf, readSize);
  if (writeSize != readSize) {
    printf("Can not write to the file\n");
    return -1;
  }

  close(output_fd);
  return 0;
}


