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

  printf("Connected to server as receiver.\n");

  while (1) {
    // Read message from server.
    int bytes_received = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
      break;
    }

    // Print message to terminal.
    buffer[bytes_received] = '\0';
    printf("Receive: %s\n", buffer);

    // Check the message to finish the program.
    if (strcmp(buffer, "The End\n") == 0 || strcmp(buffer, "The End") == 0) {
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
