#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "general.h"

int sockfd = -1;
struct ip_mreq mreq;
struct sockaddr_in multicast_addr;

// SIGINT signal handler.
void handle_sigint(int);

// check CLI arguments.
int check_arguments(int argc, char** argv);

// initialize client.
int init_client(char* address, int port);

int main(int argc, char** argv) {
  // check arguments.
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize.
  if (init_client(argv[1], atoi(argv[2])) == -1) return 0;

  char buffer[BUFFER_SIZE];
  printf("UDP Broadcast Client listening on port %s...\n", argv[1]);

  while (1) {
    // read message.
    int read = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, 0);

    if (read < 0) {
      perror("recvfrom()");
      continue;
    }

    // print message.
    buffer[read] = 0;
    printf("Received: %s\n", buffer);
  }

  setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
  close(sockfd);
  return 0;
}

void handle_sigint(int) {
  setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
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

int init_client(char* address, int port) {
  // create socket.
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("init_client, socket()");
    return -1;
  }

  // initialize address.
  memset(&multicast_addr, 0, sizeof(multicast_addr));
  multicast_addr.sin_family = AF_INET;
  multicast_addr.sin_port = htons(port);
  multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind socket.
  if (bind(sockfd, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
    perror("init_client, bind()");
    close(sockfd);
    return -1;
  }

  // attach to multicast.
  mreq.imr_multiaddr.s_addr = inet_addr(address);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    perror("init_client, setsockopt multicast");
    close(sockfd);
    return -1;
  }

  return 0;
}
