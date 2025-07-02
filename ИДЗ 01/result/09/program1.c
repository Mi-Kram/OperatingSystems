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

//// @brief Start process to read the file and write the content to the fifo.
/// @param path Path to the file to read.
/// @param output_fd Descriptor to write the file content.
/// @return Returns -1 if something went wrong.
int readFile(char* path, int output_fd);

/// @brief Start process to read from fifo and write the content to the file.
/// @param path File path to write to.
/// @param input_fd Descriptor to read content from.
/// @return Returns -1 if something went wrong.
int writeData(char* path, int input_fd);

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
  if (initialize(argv[3], argv[4], fd) == -1) return 0;

  printf("Initialization was successful\n");

  // Read file.
  printf("Start reading from the file %s\n", argv[1]);
  if (readFile(argv[1], fd[0]) == -1) {
    printf("Reading failed\n");
    dispose(argv[3], argv[4], fd);
    return 0;
  }
  close(fd[0]);

  // Write data.
  printf("Start writing to the file %s\n", argv[2]);
  if (writeData(argv[2], fd[1]) == -1) {
    printf("Writing failed\n");
    dispose(argv[3], argv[4], fd);
    return 0;
  }

  printf("The program completed successfully\n");
  dispose(argv[3], argv[4], fd);
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

int readFile(char* path, int output_fd) {
  // Open file.
  int input_fd = open(path, O_RDONLY);
  if (input_fd == -1) {
    printf("Can not open the file\n");
    return -1;
  }

  // Read file.
  char buf[BUF_SIZE];
  int readSize = read(input_fd, buf, BUF_SIZE), writeSize;

  // Read file while it is reading.
  while (0 < readSize) {
    // Write the content to fifo.
    writeSize = write(output_fd, buf, readSize);
    if (writeSize != readSize) {
      printf("Part of read data is not transfered to processing process.\n");
    }
    readSize = read(input_fd, buf, BUF_SIZE);
  }
  
  close(input_fd);
  return 0;
}

int writeData(char* path, int input_fd) {
  // Open result file.
  int output_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (output_fd == -1) {
    printf("Can not open a file\n");
    return -1;
  }

  // Read data from fifo.
  char buf[BUF_SIZE];
  int readSize = read(input_fd, buf, BUF_SIZE), writeSize;

  // Read data from fifo while it is.
  while (0 < readSize) {
    // Write the content to the file.
    writeSize = write(output_fd, buf, readSize);
    if (writeSize != readSize) {
      printf("Part of processed data is not writted to the file.\n");
    }
    readSize = read(input_fd, buf, BUF_SIZE);
  }
  
  close(output_fd);
  return 0;
}

int createFifo(char* fifo) {
  if (access(fifo, F_OK) != -1) return 0;   // Check if fifo is already created.
  if (mkfifo(fifo, 0666) != -1) return 1;   // Create fifo.
  return access(fifo, F_OK) == -1 ? -1 : 0; // Last check if it is created.
}

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
  fd[0] = open(fifo1, O_WRONLY);
  if (fd[0] == -1) {
    printf("Can not open fifo %s\n", fifo1);
    unlink(fifo1);
    unlink(fifo2);
    return -1;
  }

  // Open fifo2.
  fd[1] = open(fifo2, O_RDONLY);
  if (fd[1] == -1) {
    printf("Can not open fifo %s\n", fifo2);
    close(fd[0]);
    unlink(fifo1);
    unlink(fifo2);
    return -1;
  }

  return 0;
}

void dispose(char* fifo1, char* fifo2, int fd[2]) {
  // Close fifo and then unlink it.
  close(fd[0]);
  close(fd[1]);
  if (disposeFifo1) unlink(fifo1);
  if (disposeFifo2) unlink(fifo2);
}
