#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "general.h"

int sock;
struct sockaddr_in server_addr;
struct ip_mreq mreq;
socklen_t server_len;
volatile sig_atomic_t running = 1;


// handle SIGINT signal
void handle_sigint(int);

// verify CLI arguments
int check_arguments(int argc, char** argv);

// initialize udp client
int init_connection(char* address, int port);

// release resources
void dispose();

int main(int argc, char* argv[]) {
  // check arguments
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize client
  if (init_connection(argv[1], atoi(argv[2])) == -1) return 0;

  srand(time(NULL));

  Message msg;
  while (running) {
    // wait for messages
    if (recvfrom(sock, &msg, sizeof(msg), 0,
                 (struct sockaddr*)&server_addr, &server_len) <= 0) break;
    if (!running || msg.from_id == EXIT_ID) break;

    // Log messages based on their source
    switch (msg.from_id) {
      case IVANOV_ID:
        printf("Ivanov stole \"%s\"\n", msg.message);
        break;

      case IVANOV_DONE_ID:
        printf("Ivanov stole everything\n");
        break;

      case PETROV_ID:
        printf("Petrov uploaded \"%s\"\n", msg.message);
        break;

      case NECHEPORUK_ID:
        printf("Necheporuk valued thing at %s\n", msg.message);
        break;

      case SERVER_ID:
        printf("SERVER: %s\n", msg.message);
        break;

      default:
        break;
    }
  }

  dispose();
  return 0;
}

void handle_sigint(int sig) {
  running = 0;
  dispose();
  exit(0);
}

int check_arguments(int argc, char** argv) {
  // check amount of arguments
  if (argc != 3) {
    printf("Usage: %s <IP> <PORT>\n", argv[0]);
    return -1;
  }

  // check port value correctness
  if (atoi(argv[2]) <= 0) {
    printf("invalid argument: port should be a positive number");
    return -1;
  }

  return 0;
}

int init_connection(char* address, int port) {
  // create socket.
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("socket() failed\n");
    return -1;
  }

  // initialize address.
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
    printf("setsockopt(SO_REUSEADDR) failed\n");
    close(sock);
    return -1;
  }

  // bind socket.
  if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind() failed\n");
    close(sock);
    return -1;
  }

  // attach to multicast.
  mreq.imr_multiaddr.s_addr = inet_addr(address);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    printf("setsockopt() failed\n");
    close(sock);
    return -1;
  }

  server_len = sizeof(server_addr);
  return 0;
}

void dispose() {
  if (sock != -1) {
    close(sock);
    sock = -1;
  }
}
