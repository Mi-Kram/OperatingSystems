#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 64  // Limited buffer size to test large files

// Function to copy a file from source to destination while preserving permissions
void copy_file(const char *src, const char *dest) {
  // Open the source file for reading
  int src_fd = open(src, O_RDONLY);
  if (src_fd == -1) {
    printf("Error opening source file\n");
    return;
  }

  struct stat file_stat;
  // Get the file status to retrieve permissions
  if (fstat(src_fd, &file_stat) == -1) {
    printf("Error retrieving file information\n");
    close(src_fd);
    return;
  }

  // Open the destination file for writing, creating it if necessary
  int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, file_stat.st_mode & 0777);
  if (dest_fd == -1) {
    printf("Error opening destination file\n");
    close(src_fd);
    return;
  }

  char buffer[BUFFER_SIZE];
  long bytes_read, bytes_written;

  // Read from source and write to destination using a limited buffer
  while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
    char *ptr = buffer;
    while (bytes_read > 0) {
      bytes_written = write(dest_fd, ptr, bytes_read);
      if (bytes_written == -1) {
        printf("Error writing to file\n");
        close(src_fd);
        close(dest_fd);
        return;
      }
      bytes_read -= bytes_written;
      ptr += bytes_written;
    }
  }

  if (bytes_read == -1) {
    printf("Error reading file\n");
    return;
  }

  // Set the same permissions as the source file
  if (chmod(dest, file_stat.st_mode) == -1) {
    printf("Error setting file permissions\n");
    return;
  }

  // Close file descriptors
  close(src_fd);
  close(dest_fd);
}

int main(int argc, char** argv) {
  // Check if the correct number of arguments is provided
  if (argc != 3) {
    printf("Usage: %s <source file> <destination file>\n", argv[0]);
    return 0;
  }

  // Copy the file from source to destination
  copy_file(argv[1], argv[2]);
  return 0;
}
