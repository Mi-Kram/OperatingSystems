// server.c
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int server_fd = -1, client1_fd = -1, client2_fd = -1;

void dispose();

int main(int argc, char *argv[]) {
  // Check input arguments.
  if (argc != 3) {
    printf("Usage: %s <IP> <PORT>\n", argv[0]);
    return 0;
  }

  const char* ip = argv[1];
  int port = atoi(argv[2]);

  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];

  // Create socket.
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    printf("socket() failed\n");
    return 0;
  }

  // Set option to reuse socket.
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    printf("setsockopt() failed\n");
    dispose();
    return 1;
  }

  // Initialize the struct.
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &server_addr.sin_addr);

  // Bind the server with the port.
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    printf("bind() failed\n");
    dispose();
    return 0;
  }

  // Listen to the socket.
  if (listen(server_fd, 2) == -1) {
    printf("listen() failed\n");
    dispose();
    return 0;
  }

  // Accept client-sender request.
  printf("Server is waiting for Sender...\n");
  client1_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
  if (client1_fd == -1) {
    printf("accept() Sender failed\n");
    dispose();
    return 0;
  }

  printf("Sender connected.\n");
  printf("Server is waiting for Receiver...\n");

  // Accept client-receiver request.
  client2_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
  if (client2_fd == -1) {
    printf("accept() Receiver failed\n");
    dispose();
    return 0;
  }

  printf("Receiver connected.\n");

  while (1) {
    // Read message from client.
    int bytes_received = recv(client1_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
      printf("recv() from Receiver failed\n");
      break;
    }

    buffer[bytes_received] = 0;
    printf("Received: %s\n", buffer);

    // Send a message to another client.
    if (send(client2_fd, buffer, bytes_received, 0) == -1) {
      printf("send() to Sender failed\n");
      break;
    }
    printf("Send to Sender\n");

    // Check the message to finish the program.
    if (bytes_received == 7 && strcmp(buffer, "The End") == 0) {
      printf("The End\n");
      break;
    }
  }

  printf("Close server\n");
  dispose();
  return 0;
}

void dispose() {
  shutdown(client1_fd, 2);
  shutdown(client2_fd, 2);

  close(client1_fd);
  close(client2_fd);
  close(server_fd);
}
