#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int sock_fd = -1;

void dispose();

int main(int argc, char *argv[]) {
  // Check input arguments.
  if (argc != 3) {
    printf("Usage: %s <IP> <PORT>\n", argv[0]);
    return 0;
  }

  const char *ip = argv[1];
  int port = atoi(argv[2]);

  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  // Create socket.
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    printf("socket() failed\n");
    return 0;
  }

  // Initialize the struct.
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &server_addr.sin_addr);

  // Connect to the server.
  if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    printf("connect() failed\n");
    dispose();
    return 0;
  }

  printf("Connected to server as sender.\n");

  while (1) {
    // Input data from terminal.
    printf("Enter message: ");
    if (fgets(buffer, BUFFER_SIZE - 1, stdin) == NULL) {
      break;
    }

    // Remove newline character if it exists.
    int len = strlen(buffer);
    if (len <= 0) continue;
    if (buffer[len - 1] == '\n') buffer[--len] = 0;
    if (len == 0) continue;

    // Send message to server.
    if (send(sock_fd, buffer, len, 0) == -1) {
      printf("send() failed\n");
      break;
    }

    // Check the message to finish the program.
    if (len == 7 && strcmp(buffer, "The End") == 0) {
      break;
    }
  }

  dispose();
  return 0;
}

void dispose() {
  shutdown(sock_fd, 2);
  close(sock_fd);
}