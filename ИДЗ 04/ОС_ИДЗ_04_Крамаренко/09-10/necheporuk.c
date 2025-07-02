#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "general.h"

// UDP socket and server connection info
int sock;
struct sockaddr_in server_addr;
socklen_t server_len;

// Client running flag
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

  // Setup network connection
  if (init_connection(argv[1], atoi(argv[2])) == -1) return 0;

  srand(time(NULL));

  // Register with the server
  Message reg_msg = {NECHEPORUK_ID, REGISTER_TYPE};
  if (sendto(sock, &reg_msg, sizeof(reg_msg), 0, (struct sockaddr*)&server_addr, server_len) <= 0) {
    printf("Failed to send registration message\n");
    dispose();
    return -1;
  }

  int total = 0;
  Message income_msg;

  while (running) {
    // Wait for server command
    if (recvfrom(sock, &income_msg, sizeof(income_msg), 0, (struct sockaddr*)&server_addr, &server_len) <= 0) break;
    if (!running) break;

    // Verify message source
    if (income_msg.from_id != SERVER_ID) {
      printf("unauthorized message with id %d. Expected message from server\n", income_msg.from_id);
      break;
    }

    if (income_msg.type_id == EXIT_TYPE) break;

    // Simulate work delay
    sleep(rand() % 3 + 1);
    if (!running) break;

    // generate message
    int price = rand() % 1000 + 100;
    Message msg = {NECHEPORUK_ID, COMMON_TYPE};
    sprintf(msg.message, "%d", price);

    // Report item valuation
    printf("Necheporuk: valued a \"%s\" at %d. Total cost: %d\n", income_msg.message, price, total += price);
    if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&server_addr, server_len) <= 0) break;
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
  // create UDP socket
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    printf("socket() failed\n");
    return -1;
  }

  // Setup server address
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
