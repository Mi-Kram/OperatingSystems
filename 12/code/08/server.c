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

// initialize server.
int init_server(char* address, int port);

int main(int argc, char** argv) {
  // check arguments.
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize.
  if (init_server(argv[1], atoi(argv[2])) == -1) return 0;

  char buffer[BUFFER_SIZE];
  printf("UPD Broadcast Server started on port %s. Enter messages to send:\n", argv[2]);

  while (1) {
    // enter message.
    printf("> ");
    if (!fgets(buffer, BUFFER_SIZE - 1, stdin)) break;

    // delete new line character.
    int len = strlen(buffer);
    if (buffer[len - 1] == '\n') buffer[--len] = 0;

    if (!len) continue;

    // send message.
    if (sendto(sockfd, buffer, len, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
      perror("sendto()");
    }
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
  if (argc != 3) {
    printf("Usage: %s <address> <port>\n", argv[0]);
    return -1;
  }

  // check port value.
  if (atoi(argv[2]) <= 0) {
    printf("Invalid <port>\n");
    return -1;
  }

  return 0;
}

int init_server(char* address, int port) {
  // create socket.
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("init_server, socket()");
    return -1;
  }

  int broadcast = 1;
  // set broadcast option.
  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
    perror("init_server, setsockopt() broadcast");
    close(sockfd);
    return -1;
  }

  // initialie address.
  memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(port);
  broadcast_addr.sin_addr.s_addr = inet_addr(address);

  return 0;
}

