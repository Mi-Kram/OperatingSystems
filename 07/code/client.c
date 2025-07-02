#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "general.h"

// flag: state of grogram: 1 - running, 0 - stopped.
volatile sig_atomic_t running = 1;

pid_t server_pid;
data_t *data = NULL;

// handle program aborting.
void handle_sigint(int sig) {
  running = 0;

  // ask user to stop server.
  printf("Send signal to stop server? y/n:\n");

  // stop generating values.
  if (data) data->value = -1;

  char key = fgetc(stdin);
  while (key != EOF) {
    switch (key) {
      case 'y': case 'Y':
        kill(server_pid, SIGUSR1);

      case 'n': case 'N':
        key = EOF;
        break;

      default:
        key = fgetc(stdin);
        break;
    }
  }

  // ignore characters before '\n' including.
  key = fgetc(stdin);
  while (key != EOF && key != '\n') key = fgetc(stdin);
}

int main(int argc, char *argv[]) {
  // check corectness of launch.
  if (argc != 2) {
    printf("Usage: %s <server_pid>\n", argv[0]);
    return 0;
  }

  // read server pid.
  if (!sscanf(argv[1], "%d", &server_pid) || server_pid == getpid() || getpgid(server_pid) < 0) {
    printf("Wrong server pid \"%s\"\n", argv[1]);
    return 0;
  }

  // open new memory segment.
  int shmid = shmget(SHM_KEY, SHM_SIZE, 0644);
  if (shmid == -1) {
    perror("error: shmget returns -1");
    return 0;
  }

  // attach to memory segment.
  data = (data_t *)shmat(shmid, NULL, 0);
  if (data == (void *)-1) {
    perror("error: shmat returns -1");
    return 0;
  }

  // set signal handlers.
  signal(SIGINT, handle_sigint);  // stop program VARIAT 1: press CTRL+C.

  srand(time(NULL));
  printf("Client is running:\n");

  // program cycle: send numbers.
  while (running && !data->stop) {
    data->value = rand() % 1000;
    printf("stored: %d\n", data->value);
    sleep(1);
  }

  // stop generating values.
  data->value = -1;

  // Detach from memory segment.
  shmdt(data);

  printf("Client is finished.\n");
  return 0;
}
