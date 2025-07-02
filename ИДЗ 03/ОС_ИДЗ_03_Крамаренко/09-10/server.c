#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "general.h"


#define MAX_LOGGERS_COUNT 128

// sockets
int server_sock = -1, ivanov_sock = -1, petrov_sock = -1, necheporuk_sock = -1;
int loggers_sock[MAX_LOGGERS_COUNT];

volatile sig_atomic_t running = 1;

int stolen_things = 0, total_cost = 0;
int is_required_clients = 0;

int ivanov_command(Message* prev, Message* next);
int petrov_command(Message* prev, Message* next);
int necheporuk_command(Message* prev, Message* next);

// consecutive commands to excute.
#define COMMANDS_COUNT 3
int (*commands[COMMANDS_COUNT])(Message*, Message*) = {&ivanov_command,  &petrov_command,    &necheporuk_command};

// verify CLI arguments.
int check_arguments(int argc, char** argv);

// initialize tcp server.
int init_server(char* address, int port);

// release resources.
void dispose();

// handle SIGINT signal.
void handle_sigint(int);

// check required clients connection.
int has_required_clients();

// on clients sock changed.
void socks_updated();

// attach client.
int attach_required_clients();

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

  int cur_command = 0, is_operation_started = 0;
  Message a, b;
  Message *prev = &a, *next = &b, *tmp;

  while (running) {
    // make sure required clients to be connected and accept queue to be empty.
    if (attach_required_clients() <= -1) continue;

    if (!is_operation_started) {
      is_operation_started = 1;
      printf("start operation\n");
    }

    // execute command.
    if (commands[cur_command](prev, next) <= -1) continue;

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

int ivanov_command(Message* prev, Message* next) {
  prev->from_id = SERVER_ID;

  // send signal to ivanov to work.
  if (send(ivanov_sock, prev, sizeof(*prev), 0) <= 0) {
    printf("send() to ivanov failed\n");
    close(ivanov_sock);
    ivanov_sock = -1;
    socks_updated();
    return -1;
  }

  // get ivanov result.
  if (recv(ivanov_sock, next, sizeof(*next), 0) <= 0) {
    printf("recv() from ivanov failed\n");
    close(ivanov_sock);
    ivanov_sock = -1;
    socks_updated();
    return -1;
  }

  if (!running) return -2;

  // check response.
  if (next->from_id != IVANOV_ID && next->from_id != IVANOV_DONE_ID) {
    printf("unauthorized message with id %d. Expected message from ivanov\n", next->from_id);
    close(ivanov_sock);
    ivanov_sock = -1;
    socks_updated();
    return -1;
  }

  // send message to logger.
  log_message(next);

  // check if ivanov finished his work.
  if (next->from_id == IVANOV_DONE_ID) running = 0;
  return 0;
}

int petrov_command(Message* prev, Message* next) {
  prev->from_id = SERVER_ID;

  // send signal to petrov to work.
  if (send(petrov_sock, prev, sizeof(*prev), 0) <= 0) {
    printf("send() to petrov failed\n");
    close(petrov_sock);
    petrov_sock = -1;
    socks_updated();
    return -1;
  }

  // get petrov result.
  if (recv(petrov_sock, next, sizeof(*next), 0) <= 0) {
    printf("recv() from petrov failed\n");
    close(petrov_sock);
    petrov_sock = -1;
    socks_updated();
    return -1;
  }

  if (!running) return -2;

  // check response.
  if (next->from_id != PETROV_ID) {
    printf("unauthorized message with id %d. Expected message from petrov\n", next->from_id);
    close(petrov_sock);
    petrov_sock = -1;
    socks_updated();
    return -1;
  }

  // send message to logger.
  log_message(next);
  return 0;
}

int necheporuk_command(Message* prev, Message* next) {
  prev->from_id = SERVER_ID;

  // send signal to necheporuk to work.
  if (send(necheporuk_sock, prev, sizeof(*prev), 0) <= 0) {
    printf("send() to necheporuk failed\n");
    close(necheporuk_sock);
    necheporuk_sock = -1;
    socks_updated();
    return -1;
  }

  // get necheporuk result.
  if (recv(necheporuk_sock, next, sizeof(*next), 0) <= 0) {
    printf("recv() from necheporuk failed\n");
    close(necheporuk_sock);
    necheporuk_sock = -1;
    socks_updated();
    return -1;
  }

  if (!running) return -2;

  // check response.
  if (next->from_id != NECHEPORUK_ID) {
    printf("unauthorized message with id %d. Expected message from necheporuk\n", next->from_id);
    close(necheporuk_sock);
    necheporuk_sock = -1;
    socks_updated();
    return -1;
  }

  // send message to logger.
  log_message(next);

  ++stolen_things;
  total_cost += atoi(next->message);
  return 0;
}

int has_required_clients() {
  return (ivanov_sock != -1 && petrov_sock != -1 && necheporuk_sock != -1) ? 0 : -1;
}

void socks_updated() {
  // check required clients to change blocking accept().
  int required = has_required_clients();

  if (is_required_clients != required) {
    // get existing flags.
    int flags = fcntl(server_sock, F_GETFL);
    if (flags == -1) {
      printf("fcntl() F_GETFL failed\n");
      return;
    }

    // update flags.
    if (required == -1) flags &= ~O_NONBLOCK;
    else flags |= O_NONBLOCK;

    // set new flags.
    if (fcntl(server_sock, F_SETFL, flags) == -1) {
      printf("fcntl() failed. required_clients = %d\n", required);
      return;
    }

    is_required_clients = required;
  }
}

int attach_required_clients() {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  // wait for connection.
  int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
  if (client_sock == -1) return has_required_clients();

  // receive client id.
  int client_id;
  if (recv(client_sock, &client_id, sizeof(int), 0) <= 0) {
    close(client_sock);
    return -2;
  }

  // recognize client id.
  switch (client_id) {
    case IVANOV_ID:
      if (ivanov_sock != -1) close(client_sock);
      else {
        ivanov_sock = client_sock;
        printf("ivanov connected\n");
      }
      break;
    case PETROV_ID:
      if (petrov_sock != -1) close(client_sock);
      else {
        petrov_sock = client_sock;
        printf("petrov connected\n");
      }
      break;
    case NECHEPORUK_ID:
      if (necheporuk_sock != -1) close(client_sock);
      else {
        necheporuk_sock = client_sock;
        printf("necheporuk connected\n");
      }
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

  socks_updated();
  return -2;
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
