#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "general.h"

int sockfd = -1;
struct sockaddr_in multicast_addr;

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
  printf("Multicast server started. Type messages to send:\n");

  while (1) {
    printf("> ");
    if (!fgets(buffer, BUFFER_SIZE - 1, stdin)) break;

    // delete new line character.
    int len = strlen(buffer);
    if (buffer[len - 1] == '\n') buffer[--len] = 0;

    if (!len) continue;

    // send message.
    if (sendto(sockfd, buffer, len, 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
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

  // Настройка адреса для multicast
  memset(&multicast_addr, 0, sizeof(multicast_addr));
  multicast_addr.sin_family = AF_INET;
  multicast_addr.sin_port = htons(port);
  multicast_addr.sin_addr.s_addr = inet_addr(address);

  // set multicast option.
  unsigned char ttl = 1;
  if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
    perror("init_server, setsockopt multicast");
    close(sockfd);
    return -1;
  }

  return 0;
}
