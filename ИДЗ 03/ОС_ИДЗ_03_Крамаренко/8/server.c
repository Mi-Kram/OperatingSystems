#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "general.h"

#define MAX_LOGGERS_COUNT 128

// sockets
int server_sock = -1, ivanov_sock = -1, petrov_sock = -1, necheporuk_sock = -1;
int loggers_sock[MAX_LOGGERS_COUNT];

volatile sig_atomic_t running = 1;

int stolen_things = 0, total_cost = 0;

void request_ivanov(Message* prev, Message* next);
void response_ivanov(Message* prev, Message* next);

void request_petrov(Message* prev, Message* next);
void response_petrov(Message* prev, Message* next);

void request_necheporuk(Message* prev, Message* next);
void response_necheporuk(Message* prev, Message* next);

// consecutive commands to excute.
#define COMMANDS_COUNT 6
void (*commands[COMMANDS_COUNT])(Message*, Message*) = {
  &request_ivanov,
  &response_ivanov,
  &request_petrov,
  &response_petrov,
  &request_necheporuk,
  &response_necheporuk
};

// verify CLI arguments.
int check_arguments(int argc, char** argv);

// initialize tcp server.
int init_server(char* address, int port);

// connects clients.
void connect_clients();

// release resources.
void dispose();

// handle SIGINT signal.
void handle_sigint(int);

// attach logger client.
int attach_logger();

// send log to loggers.
void log_message(Message* msg);

// add client logger.
int add_logger(int client_sock);

// remove client logger.
int remove_logger(int client_sock);

int main(int argc, char* argv[]) {
  // check arguments.
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize server.
  if (init_server(argv[1], atoi(argv[2])) == -1) return 0;
  printf("Server listening on %s:%s\n", argv[1], argv[2]);

  // connect clients.
  connect_clients();

  // make accept not blocking.
  if (fcntl(server_sock, F_SETFL, O_NONBLOCK) == -1) {
    printf("fcntl() O_NONBLOCK failed\n");
    dispose();
    return -1;
  }

  printf("start operation\n");

  int cur_command = 0;
  Message a, b;
  Message *prev = &a, *next = &b, *tmp;

  while (running) {
    // if new logger accepted iterate again.
    int attach_res = attach_logger();
    if (attach_res < -1) continue;
    if (attach_res != -1) {
      printf("logger connected\n");
      continue;
    }

    // execute command.
    commands[cur_command](prev, next);

    // swap messages.
    tmp = prev;
    prev = next;
    next = tmp;

    // shift pointer to next command.
    if (++cur_command == COMMANDS_COUNT) cur_command = 0;
  }
  
  printf("end operation\n");


  // summary.
  Message msg = {SERVER_ID};

  sprintf(msg.message, "stolen: %d %s", stolen_things, stolen_things == 1 ? "thing" : "things");
  printf("%s\n", msg.message);
  log_message(&msg);

  sprintf(msg.message, "total cost: %d", total_cost);
  printf("%s\n", msg.message);
  log_message(&msg);

  dispose();
  return 0;
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

int init_server(char* address, int port) {
  // Create socket.
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock == -1) {
    printf("socket() failed\n");
    return -1;
  }

  // Set option to reuse socket.
  int opt = 1;
  if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    printf("setsockopt() failed\n");
    close(server_sock);
    return -1;
  }

  struct sockaddr_in server_addr, client_addr;

  // Initialize the struct.
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, address, &server_addr.sin_addr) == -1) {
    printf("inet_pton() failed\n");
    close(server_sock);
    return -1;
  }

  // Bind the server with the port.
  if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    printf("bind() failed\n");
    close(server_sock);
    return -1;
  }

  // initialize loggers.
  for (int i = 0; i < MAX_LOGGERS_COUNT; ++i) loggers_sock[i] = -1;

  // Listen to the socket.
  if (listen(server_sock, 5) == -1) {
    printf("listen() failed\n");
    close(server_sock);
    return -1;
  }

  return 0;
}

void connect_clients() {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  // wait for ivanov, petrov and nechepurok connection.
  while (ivanov_sock == -1 || petrov_sock == -1 || necheporuk_sock == -1) {
    // wait for connection.
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock == -1) continue;

    // receive client id.
    int client_id;
    if (recv(client_sock, &client_id, sizeof(int), 0) <= 0) {
      close(client_sock);
      continue;
    }

    // recognize client id.
    switch (client_id) {
      case IVANOV_ID:
        if (ivanov_sock != -1) close(ivanov_sock);
        ivanov_sock = client_sock;
        printf("ivanov connected\n");
        break;
      case PETROV_ID:
        if (petrov_sock != -1) close(petrov_sock);
        petrov_sock = client_sock;
        printf("petrov connected\n");
        break;
      case NECHEPORUK_ID:
        if (necheporuk_sock != -1) close(necheporuk_sock);
        necheporuk_sock = client_sock;
        printf("necheporuk connected\n");
        break;
      case LOGGER_ID:
        if (add_logger(client_sock) == -1) close(client_sock);
        else printf("logger connected\n");        
        break;

      default:
        printf("unauthorized connection: client_id = %d\n", client_id);
        close(client_sock);
        break;
    }
  }
}

void request_ivanov(Message* prev, Message* next) {
  prev->from_id = SERVER_ID;

  // send signal to ivanov to work.
  if (send(ivanov_sock, prev, sizeof(*prev), 0) <= 0) {
    printf("send() to ivanov failed\n");
    running = 0;
  }
}

void response_ivanov(Message* prev, Message* next) {
  // get ivanov result.
  if (recv(ivanov_sock, next, sizeof(*next), 0) <= 0) {
    printf("recv() from ivanov failed\n");
    running = 0;
    return;
  }

  if (!running) return;

  // check response.
  if (next->from_id != IVANOV_ID && next->from_id != IVANOV_DONE_ID) {
    printf("unauthorized message with id %d. Expected message from ivanov\n", next->from_id);
    running = 0;
    return;
  }

  // send message to logger.
  log_message(next);

  // check if ivanov finished his work.
  if (next->from_id == IVANOV_DONE_ID) running = 0;
}

void request_petrov(Message* prev, Message* next) {
  prev->from_id = SERVER_ID;

  // send signal to petrov to work.
  if (send(petrov_sock, prev, sizeof(*prev), 0) <= 0) {
    printf("send() to petrov failed\n");
    running = 0;
  }
}

void response_petrov(Message* prev, Message* next) {
  // get petrov result.
  if (recv(petrov_sock, next, sizeof(*next), 0) <= 0) {
    printf("recv() from petrov failed\n");
    running = 0;
    return;
  }

  if (!running) return;

  // check response.
  if (next->from_id != PETROV_ID) {
    printf("unauthorized message with id %d. Expected message from petrov\n", next->from_id);
    running = 0;
    return;
  }

  // send message to logger.
  log_message(next);
}

void request_necheporuk(Message* prev, Message* next) {
  prev->from_id = SERVER_ID;

  // send signal to necheporuk to work.
  if (send(necheporuk_sock, prev, sizeof(*prev), 0) <= 0) {
    printf("send() to necheporuk failed\n");
    running = 0;
    return;
  }
}

void response_necheporuk(Message* prev, Message* next) {
  // get necheporuk result.
  if (recv(necheporuk_sock, next, sizeof(*next), 0) <= 0) {
    printf("recv() from necheporuk failed\n");
    running = 0;
    return;
  }

  if (!running) return;

  // check response.
  if (next->from_id != NECHEPORUK_ID) {
    printf("unauthorized message with id %d. Expected message from necheporuk\n", next->from_id);
    running = 0;
    return;
  }

  // send message to logger.
  log_message(next);

  ++stolen_things;
  total_cost += atoi(next->message);
}


int attach_logger() {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  // wait for connection.
  int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
  if (client_sock == -1) return -1;

  // receive client id.
  int client_id;
  if (recv(client_sock, &client_id, sizeof(int), 0) <= 0) {
    close(client_sock);
    return -2;
  }

  // check client id.
  if (client_id != LOGGER_ID) {
    close(client_sock);
    return -3;
  }

  // try add logger.
  if (add_logger(client_sock) == -1) {
    close(client_sock);
    return -4;
  }

  return 0;
}

void log_message(Message* msg) {
  Message response;

  for (int i = 0; i < MAX_LOGGERS_COUNT; ++i) {
    if (loggers_sock[i] == -1) continue;

    // send log
    if (send(loggers_sock[i], msg, sizeof(*msg), 0) <= 0) {
      remove_logger(loggers_sock[i]);
      continue;
    }

    // response means that loggoer is still connected.
    if (recv(loggers_sock[i], &response, sizeof(response), 0) <= 0) {
      remove_logger(loggers_sock[i]);
    }
  }
}

int add_logger(int client_sock) {
  for (int i = 0; i < MAX_LOGGERS_COUNT; ++i) {
    if (loggers_sock[i] == -1) {
      loggers_sock[i] = client_sock;
      return 0;
    }
  }

  return -1;
}

int remove_logger(int client_sock) {
  for (int i = 0; i < MAX_LOGGERS_COUNT; ++i) {
    if (loggers_sock[i] == client_sock) {
      close(loggers_sock[i]);
      loggers_sock[i] = -1;
      return 0;
    }
  }

  return -1;
}

void dispose() {
  running = 0;

  for (int i = 0; i < MAX_LOGGERS_COUNT; ++i) close(loggers_sock[i]);

  close(ivanov_sock);
  close(petrov_sock);
  close(necheporuk_sock);
  close(server_sock);
}

void handle_sigint(int) {
  printf("\nSIGINT caught: stop working\n");
  dispose();
  exit(0);
}
