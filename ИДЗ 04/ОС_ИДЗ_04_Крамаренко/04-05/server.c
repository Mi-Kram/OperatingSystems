#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "general.h"

// UDP socket
int server_sock = -1;
volatile sig_atomic_t running = 1;

// Client addresses
struct sockaddr_in ivanov_addr, petrov_addr, necheporuk_addr;
int ivanov_ready = 0, petrov_ready = 0, necheporuk_ready = 0;

int stolen_things = 0, total_cost = 0;

// verify CLI arguments
int check_arguments(int argc, char** argv);

// initialize udp server
int init_server(char* address, int port);

// wait for initial messages from clients
void wait_for_clients();

// manage clients work
void orchestrate();

// release resources
void dispose();

// handle SIGINT signal
void handle_sigint(int);

int main(int argc, char* argv[]) {
  // check arguments
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize server
  if (init_server(argv[1], atoi(argv[2])) == -1) return 0;
  printf("Server listening on %s:%s\n", argv[1], argv[2]);

  // wait for initial messages from clients
  wait_for_clients();

  // start work
  printf("start operation\n");
  orchestrate();
  printf("end operation\n");

  // summary
  printf("stolen: %d %s\n", stolen_things, stolen_things == 1 ? "thing" : "things");
  printf("total cost: %d\n", total_cost);

  dispose();
  return 0;
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

int init_server(char* address, int port) {
  // Create UDP socket
  server_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_sock == -1) {
    printf("socket() failed\n");
    return -1;
  }

  struct sockaddr_in server_addr;

  // Initialize the struct
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, address, &server_addr.sin_addr) == -1) {
    printf("inet_pton() failed\n");
    dispose();
    return -1;
  }

  // Bind the server with the port
  if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    printf("bind() failed\n");
    dispose();
    return -1;
  }

  return 0;
}

void wait_for_clients() {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  Message msg;

  // wait for initial messages from all clients
  while (!ivanov_ready || !petrov_ready || !necheporuk_ready) {
    memset(&msg, 0, sizeof(Message));
    
    // receive message from any client
    if (recvfrom(server_sock, &msg, sizeof(Message), 0, 
                 (struct sockaddr*)&client_addr, &client_len) <= 0) {
      continue;
    }

    // store client address based on ID
    switch (msg.from_id) {
      case IVANOV_ID:
        if (!ivanov_ready) {
          memcpy(&ivanov_addr, &client_addr, sizeof(struct sockaddr_in));
          ivanov_ready = 1;
          printf("ivanov registered\n");
        }
        break;
      case PETROV_ID:
        if (!petrov_ready) {
          memcpy(&petrov_addr, &client_addr, sizeof(struct sockaddr_in));
          petrov_ready = 1;
          printf("petrov registered\n");
        }
        break;
      case NECHEPORUK_ID:
        if (!necheporuk_ready) {
          memcpy(&necheporuk_addr, &client_addr, sizeof(struct sockaddr_in));
          necheporuk_ready = 1;
          printf("necheporuk registered\n");
        }
        break;
      default:
        printf("unauthorized message from client_id = %d\n", msg.from_id);
        break;
    }
  }
}

void orchestrate() {
  Message msg;
  socklen_t addr_len = sizeof(struct sockaddr_in);

  while (running) {
    msg.from_id = SERVER_ID;
    memset(msg.message, 0, MAX_MSG_LEN);

    // send signal to ivanov to work
    if (sendto(server_sock, &msg, sizeof(msg), 0,
               (struct sockaddr*)&ivanov_addr, addr_len) <= 0) {
      printf("sendto() to ivanov failed\n");
      break;
    }

    // get ivanov result
    if (recvfrom(server_sock, &msg, sizeof(msg), 0,
                 (struct sockaddr*)&ivanov_addr, &addr_len) <= 0) {
      printf("recvfrom() from ivanov failed\n");
      break;
    }

    if (!running) break;

    // check response
    if (msg.from_id != IVANOV_ID && msg.from_id != IVANOV_DONE_ID) {
      printf("unauthorized message with id %d. Expected message from ivanov\n", msg.from_id);
      break;
    }

    // check if ivanov finished his work
    if (msg.from_id == IVANOV_DONE_ID) {
      sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr*)&petrov_addr, addr_len);
      sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr*)&necheporuk_addr, addr_len);
      break;
    }

    msg.from_id = SERVER_ID;

    // send signal to petrov to work
    if (sendto(server_sock, &msg, sizeof(msg), 0,
               (struct sockaddr*)&petrov_addr, addr_len) <= 0) {
      printf("sendto() to petrov failed\n");
      break;
    }

    // get petrov result
    if (recvfrom(server_sock, &msg, sizeof(msg), 0,
                 (struct sockaddr*)&petrov_addr, &addr_len) <= 0) {
      printf("recvfrom() from petrov failed\n");
      break;
    }

    if (!running) break;

    // check response
    if (msg.from_id != PETROV_ID) {
      printf("unauthorized message with id %d. Expected message from petrov\n", msg.from_id);
      break;
    }

    msg.from_id = SERVER_ID;

    // send signal to necheporuk to work
    if (sendto(server_sock, &msg, sizeof(msg), 0,
               (struct sockaddr*)&necheporuk_addr, addr_len) <= 0) {
      printf("sendto() to necheporuk failed\n");
      break;
    }

    // get necheporuk result
    if (recvfrom(server_sock, &msg, sizeof(msg), 0,
                 (struct sockaddr*)&necheporuk_addr, &addr_len) <= 0) {
      printf("recvfrom() from necheporuk failed\n");
      break;
    }

    if (!running) break;

    // check response
    if (msg.from_id != NECHEPORUK_ID) {
      printf("unauthorized message with id %d. Expected message from necheporuk\n", msg.from_id);
      break;
    }

    ++stolen_things;
    total_cost += atoi(msg.message);
  }
}

void dispose() {
  running = 0;
  if (server_sock != -1) {
    close(server_sock);
    server_sock = -1;
  }
}

void handle_sigint(int) {
  printf("\nSIGINT caught: stop working\n");
  dispose();
  exit(0);
}
