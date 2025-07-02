#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "general.h"

int sock;
int cur = 0, n = 0;
volatile sig_atomic_t running = 1;

// handle SIGINT signal.
void handle_sigint(int);

// verify CLI arguments.
int check_arguments(int argc, char** argv);

// initialize tcp client.
int init_connection(char* address, int port);

// release resources.
void dispose();

int main(int argc, char* argv[]) {
  // check arguments.
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize client.
  if (init_connection(argv[1], atoi(argv[2])) == -1) return 0;

  // get amount of things to steal.
  n = argc == 4 ? atoi(argv[3]) : 10;
  srand(time(NULL));

  Message income_msg;
  while (running && ++cur <= n) {
    // wait for server signal.
    if (recv(sock, &income_msg, sizeof(income_msg), 0) <= 0) break;
    if (!running) break;

    // check response.
    if (income_msg.from_id != SERVER_ID) {
      printf("unauthorized message with id %d. Expected message from server\n", income_msg.from_id);
      break;
    }

    // imitate work.
    sleep(rand() % 3 + 1);
    if (!running) break;

    // generate message.
    Message msg = {IVANOV_ID};
    int written = snprintf(msg.message, MAX_MSG_LEN - 1, "Box #%d", cur);
    msg.message[written] = 0;

    // print and send result.
    printf("Ivanov stole \"%s\"\n", msg.message);
    if (send(sock, &msg, sizeof(msg), 0) <= 0) break;
  }

  if (n < cur) {
    printf("Ivanov stole everything\n");
    Message final_msg = {IVANOV_DONE_ID};
    send(sock, &final_msg, sizeof(final_msg), 0);
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
  if (argc != 3 && argc != 4) {
    printf("Usage: %s <IP> <PORT>\n", argv[0]);
    return -1;
  }

  // check port value correctness.
  if (atoi(argv[2]) <= 0) {
    printf("invalid argument: port should be a positive number");
    return -1;
  }

  // check forth argument: amount of things to steal
  if (argc == 4) {
    int n = atoi(argv[3]);
    if (n <= 0) {
      printf("invalid amount of things to steal");
      return -1;
    }
  }

  return 0;
}

int init_connection(char* address, int port) {
  // create socket.
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("socket() failed\n");
    return -1;
  }

  struct sockaddr_in addr;

  // initialize the struct.
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, address, &addr.sin_addr) == -1) {
    printf("inet_pton() failed\n");
    dispose();
    return -1;
  }

  // connect to the server.
  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    printf("connect() failed\n");
    dispose();
    return -1;
  }

  // connect ivanov client id.
  int id = IVANOV_ID;
  if (send(sock, &id, sizeof(id), 0) <= 0) {
    printf("send id faield\n");
    dispose();
    return -1;
  }

  return 0;
}

void dispose() {
  close(sock);
}
