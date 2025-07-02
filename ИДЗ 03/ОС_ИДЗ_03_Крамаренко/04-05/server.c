#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "general.h"

// sockets
int server_sock = -1, ivanov_sock = -1, petrov_sock = -1, necheporuk_sock = -1;
volatile sig_atomic_t running = 1;

int stolen_things = 0, total_cost = 0;

// verify CLI arguments.
int check_arguments(int argc, char** argv);

// initialize tcp server.
int init_server(char* address, int port);

// connects clients.
void connect_clients();

// manage clients work.
void orchestrate();

// release resources.
void dispose();

// handle SIGINT signal.
void handle_sigint(int);

int main(int argc, char* argv[]) {
  // check arguments.
  if (check_arguments(argc, argv) == -1) return 0;
  signal(SIGINT, handle_sigint);

  // initialize server.
  if (init_server(argv[1], atoi(argv[2])) == -1) return 0;
  printf("Server listening on %s:%s\n", argv[1], argv[2]);

  // connect clients.
  connect_clients();

  // start work.
  printf("start operation\n");
  orchestrate();
  printf("end operation\n");

  // summary.
  printf("stolen: %d %s\n", stolen_things, stolen_things == 1 ? "thing" : "things");
  printf("total cost: %d\n", total_cost);

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
    dispose();
    return -1;
  }

  struct sockaddr_in server_addr, client_addr;

  // Initialize the struct.
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, address, &server_addr.sin_addr) == -1) {
    printf("inet_pton() failed\n");
    dispose();
    return -1;
  }

  // Bind the server with the port.
  if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    printf("bind() failed\n");
    dispose();
    return -1;
  }

  // Listen to the socket.
  if (listen(server_sock, 3) == -1) {
    printf("listen() failed\n");
    dispose();
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

      default:
        printf("unauthorized connection: client_id = %d\n", client_id);
        close(client_sock);
        break;
    }
  }
}

void orchestrate() {
  while (running) {
    Message msg = {SERVER_ID};

    // send signal to ivanov to work.
    if (send(ivanov_sock, &msg, sizeof(msg), 0) <= 0) {
      printf("send() to ivanov failed\n");
      break;
    }

    // get ivanov result.
    if (recv(ivanov_sock, &msg, sizeof(msg), 0) <= 0) {
      printf("recv() from ivanov failed\n");
      break;
    }

    if (!running) break;

    // check response.
    if (msg.from_id != IVANOV_ID && msg.from_id != IVANOV_DONE_ID) {
      printf("unauthorized message with id %d. Expected message from ivanov\n", msg.from_id);
      break;
    }

    // check if ivanov finished his work.
    if (msg.from_id == IVANOV_DONE_ID) break;

    msg.from_id = SERVER_ID;

    // send signal to petrov to work.
    if (send(petrov_sock, &msg, sizeof(msg), 0) <= 0) {
      printf("send() to petrov failed\n");
      break;
    }

    // get petrov result.
    if (recv(petrov_sock, &msg, sizeof(msg), 0) <= 0) {
      printf("recv() from petrov failed\n");
      break;
    }

    if (!running) break;

    // check response.
    if (msg.from_id != PETROV_ID) {
      printf("unauthorized message with id %d. Expected message from petrov\n", msg.from_id);
      break;
    }

    msg.from_id = SERVER_ID;

    // send signal to necheporuk to work.
    if (send(necheporuk_sock, &msg, sizeof(msg), 0) <= 0) {
      printf("send() to necheporuk failed\n");
      break;
    }

    // get necheporuk result.
    if (recv(necheporuk_sock, &msg, sizeof(msg), 0) <= 0) {
      printf("recv() from necheporuk failed\n");
      break;
    }

    if (!running) break;

    // check response.
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
