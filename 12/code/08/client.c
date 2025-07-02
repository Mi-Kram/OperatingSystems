#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "general.h"

int sockfd = -1;
struct sockaddr_in broadcast_addr;

// SIGINT signal handler.
void handle_sigint(int);

// check CLI arguments.
int check_arguments(int argc, char** argv);

// initialize client.
int init_client(int port);

int main(int argc, char** argv) {
  // check arguments.
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize.
  if (init_client(atoi(argv[1])) == -1) return 0;

  char buffer[BUFFER_SIZE];
  printf("UDP Broadcast Client listening on port %s...\n", argv[1]);

  while (1) {
    // read message.
    int read = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, 0);

    if (read < 0) {
      perror("recvfrom");
      continue;
    }

    // print message.
    buffer[read] = 0;
    printf("Received: %s\n", buffer);
  }

  close(sockfd);
  return 0;
}

void handle_sigint(int) {
  close(sockfd);
  exit(0);
}

int check_arguments(int argc, char** argv) {
  // check amount of arguments.
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return -1;
  }

  // check port value.
  if (atoi(argv[1]) <= 0) {
    printf("Invalid <port>\n");
    return -1;
  }

  return 0;
}

int init_client(int port) {
  // create socket.
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("init_client, socket()");
    return -1;
  }
  
  // initialize address.
  memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(port);
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind socket.
  if (bind(sockfd, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
    perror("init_client, bind()");
    close(sockfd);
    return -1;
  }

  return 0;
}

