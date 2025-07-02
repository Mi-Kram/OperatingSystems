#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "general.h"

// UDP socketes
int server_sock = -1, logger_sock = -1;

volatile sig_atomic_t running = 1;

// Exit message template for stopping clients
const Message msg_exit = {SERVER_ID, EXIT_TYPE};

// Client addresses
struct sockaddr_in ivanov_addr = {0}, petrov_addr = {0}, necheporuk_addr = {0}, logger_addr = {0};
int stolen_things = 0, total_cost = 0;

// verify CLI arguments
int check_arguments(int argc, char** argv);

// initialize udp server
int init_server(char* address, int port, char* logger_address, int logger_port);

// register client
void register_client(int id, struct sockaddr_in* addr);

// manage clients work
void orchestrate();

// release resources
void dispose();

// handle SIGINT signal
void handle_sigint(int);

// forward message to logger
void forward_to_logger(const Message* msg);

// handle client command
int handle_command(Message* request, Message* response, int client_id, struct sockaddr_in* addr);

int main(int argc, char* argv[]) {
  // check arguments
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize server
  if (init_server(argv[1], atoi(argv[2]), argv[3], atoi(argv[4])) == -1) return 0;
  printf("Main server listening on %s:%s\n", argv[1], argv[2]);
  printf("Logger server listening on %s:%s\n", argv[3], argv[4]);

  // start work
  printf("start operation\n");
  orchestrate();
  printf("end operation\n");

  // summary
  Message msg = {SERVER_ID, COMMON_TYPE};

  sprintf(msg.message, "stolen: %d %s", stolen_things, stolen_things == 1 ? "thing" : "things");
  printf("%s\n", msg.message);
  forward_to_logger(&msg);

  sprintf(msg.message, "total cost: %d", total_cost);
  printf("%s\n", msg.message);
  forward_to_logger(&msg);

  forward_to_logger(&msg_exit);

  dispose();
  return 0;
}

int check_arguments(int argc, char** argv) {
  // check amount of arguments
  if (argc != 5) {
    printf("Usage: %s <IP> <PORT> <LOGGER_IP> <LOGGER_PORT>\n", argv[0]);
    return -1;
  }

  // check port value correctness
  if (atoi(argv[2]) <= 0 || atoi(argv[4]) <= 0) {
    printf("invalid argument: port should be a positive number");
    return -1;
  }

  return 0;
}

int init_server(char* address, int port, char* logger_address, int logger_port) {
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

  // create socket.
  logger_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (logger_sock == -1) {
    printf("socket() failed\n");
    dispose();
    return -1;
  }

  // Initialize the struct
  memset(&logger_addr, 0, sizeof(logger_addr));
  logger_addr.sin_family = AF_INET;
  logger_addr.sin_port = htons(logger_port);
  logger_addr.sin_addr.s_addr = inet_addr(logger_address);

  // set multicast option.
  unsigned char ttl = 1;
  if (setsockopt(logger_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
    printf("setsockopt() failed\n");
    dispose();
    return -1;
  }

  return 0;
}

void register_client(int id, struct sockaddr_in* addr) {
  // store client address based on ID
  switch (id) {
    case IVANOV_ID:
      sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&ivanov_addr, sizeof(struct sockaddr_in));
      memcpy(&ivanov_addr, addr, sizeof(struct sockaddr_in));
      printf("ivanov registered\n");
      break;
    case PETROV_ID:
      sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&petrov_addr, sizeof(struct sockaddr_in));
      memcpy(&petrov_addr, addr, sizeof(struct sockaddr_in));
      printf("petrov registered\n");
      break;
    case NECHEPORUK_ID:
      sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&necheporuk_addr, sizeof(struct sockaddr_in));
      memcpy(&necheporuk_addr, addr, sizeof(struct sockaddr_in));
      printf("necheporuk registered\n");
      break;
    default:
      printf("unauthorized message from client_id = %d\n", id);
      break;
  }
}

void orchestrate() {
  Message msg_a, msg_b;
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);

  while (running) {
    msg_a.from_id = SERVER_ID;
    msg_a.type_id = COMMON_TYPE;

    // Process Ivanov's request and response
    if (handle_command(&msg_a, &msg_b, IVANOV_ID, &ivanov_addr) == -1) {
      running = -1;
      break;
    }

    // Send Ivanov's response to logger
    forward_to_logger(&msg_b);
    if (!running) break;

    // Check if Ivanov has completed his task
    if (msg_b.type_id == DONE_TYPE) {
      sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&petrov_addr, addr_len);
      sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&necheporuk_addr, addr_len);
      break;
    }

    msg_b.from_id = SERVER_ID;
    msg_b.type_id = COMMON_TYPE;

    // Process Petrov's request and response
    if (handle_command(&msg_b, &msg_a, PETROV_ID, &petrov_addr) == -1) {
      running = -1;
      break;
    }
    
    // Send Petrov's response to logger
    forward_to_logger(&msg_a);
    if (!running) break;

    msg_b.from_id = SERVER_ID;
    msg_b.type_id = COMMON_TYPE;

    // Process Necheporuk's request and response
    if (handle_command(&msg_b, &msg_a, NECHEPORUK_ID, &necheporuk_addr) == -1) {
      running = -1;
      break;
    }
    
    // Send Necheporuk's response to logger
    forward_to_logger(&msg_a);
    if (!running) break;

    // Update operation statistics
    ++stolen_things;
    total_cost += atoi(msg_a.message);
  }
}

void forward_to_logger(const Message* msg) {
  sendto(logger_sock, msg, sizeof(Message), 0, (struct sockaddr*)&logger_addr, sizeof(struct sockaddr_in));
}

int handle_command(Message* request, Message* response, int client_id, struct sockaddr_in* addr) {
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);

  // Send command to client
  sendto(server_sock, request, sizeof(*request), 0, (struct sockaddr*)addr, addr_len);

  while (1) {
    // Wait for client's response
    if (recvfrom(server_sock, response, sizeof(*response), 0, (struct sockaddr*)&client_addr, &addr_len) <= 0) {
      printf("recvfrom() failed\n");
      return -1;
    }

    // Check if server is still running
    if (!running) return -1;

    // Handle new client registration
    if (response->type_id == REGISTER_TYPE) {
      // register client.
      register_client(response->from_id, &client_addr);

      // Resend request if current client reconnected
      if (response->from_id == client_id) {
        sendto(server_sock, request, sizeof(*request), 0, (struct sockaddr*)addr, addr_len);
      }
      continue;
    }

    // Verify response is from expected client
    if (response->from_id == client_id || response->type_id == COMMON_TYPE || response->type_id == DONE_TYPE) return 0;
  }
}

void dispose() {
  running = 0;
  if (server_sock != -1) {
    close(server_sock);
    close(logger_sock);
    server_sock = -1;
    logger_sock = -1;
  }
}

void handle_sigint(int) {
  printf("\nSIGINT caught: stop working\n");

  // Notify all clients and logger about shutdown
  sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&ivanov_addr, sizeof(struct sockaddr_in));
  sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&petrov_addr, sizeof(struct sockaddr_in));
  sendto(server_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&necheporuk_addr, sizeof(struct sockaddr_in));
  sendto(logger_sock, &msg_exit, sizeof(msg_exit), 0, (struct sockaddr*)&logger_addr, sizeof(struct sockaddr_in));

  dispose();
  exit(0);
}
