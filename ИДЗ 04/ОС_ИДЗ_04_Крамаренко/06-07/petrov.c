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
socklen_t server_len;
volatile sig_atomic_t running = 1;

// handle SIGINT signal.
void handle_sigint(int);

// verify CLI arguments.
int check_arguments(int argc, char** argv);

// initialize udp client.
int init_connection(char* address, int port);

// release resources.
void dispose();

int main(int argc, char* argv[]) {
  // check arguments.
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize client.
  if (init_connection(argv[1], atoi(argv[2])) == -1) return 0;

  srand(time(NULL));

  // send initial registration message
  Message reg_msg = {PETROV_ID};
  if (sendto(sock, &reg_msg, sizeof(reg_msg), 0,
             (struct sockaddr*)&server_addr, server_len) <= 0) {
    printf("Failed to send registration message\n");
    dispose();
    return -1;
  }

  Message income_msg;
  while (running) {
    // wait for server signal.
    if (recvfrom(sock, &income_msg, sizeof(income_msg), 0,
                 (struct sockaddr*)&server_addr, &server_len) <= 0) break;
    if (!running) break;

    // check response.
    if (income_msg.from_id == EXIT_ID) break;

    if (income_msg.from_id != SERVER_ID) {
      printf("unauthorized message with id %d. Expected message from server\n", income_msg.from_id);
      break;
    }

    // imitate work.
    sleep(rand() % 3 + 1);
    if (!running) break;

    // generate message.
    income_msg.from_id = PETROV_ID;
    printf("Petrov uploaded \"%s\"\n", income_msg.message);
    if (sendto(sock, &income_msg, sizeof(income_msg), 0,
               (struct sockaddr*)&server_addr, server_len) <= 0) break;
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
  // check amount of arguments.
  if (argc != 3) {
    printf("Usage: %s <IP> <PORT>\n", argv[0]);
    return -1;
  }

  // check port value correctness.
  if (atoi(argv[2]) <= 0) {
    printf("invalid argument: port should be a positive number");
    return -1;
  }

  return 0;
}

int init_connection(char* address, int port) {
  // create UDP socket.
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    printf("socket() failed\n");
    return -1;
  }

  // initialize the server address struct.
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, address, &server_addr.sin_addr) == -1) {
    printf("inet_pton() failed\n");
    dispose();
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
