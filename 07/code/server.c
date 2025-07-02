#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "general.h"

// flag: state of grogram: 1 - running, 0 - stopped.
volatile sig_atomic_t running = 1;

// handle program aborting.
void stop_running(int) {
  running = 0;
}

int main(int argc, char *argv[]) {
  // create new memory segment.
  int shmid = shmget(SHM_KEY, SHM_SIZE, 0644 | IPC_CREAT | IPC_EXCL);
  if (shmid == -1) {
    perror("error: shmget returns -1");
    return 0;
  }

  // attach to memory segment.
  data_t *data = (data_t *)shmat(shmid, NULL, 0);
  if (data == (void *)-1) {
    perror("error: shmat returns -1");
    return 0;
  }

  // set default values.
  data->value = -1;
  data->stop = 0;

  printf("Server pid: %d\n", getpid());

  // set signal handlers.
  signal(SIGINT, stop_running);   // stop program VARIAT 1: press CTRL+C.
  signal(SIGUSR1, stop_running);  // stop program VARIAT 2: message from client.
  printf("Server is running:\n");

  // program cycle: receive numbers.
  while (running) {
    // if number is set print it.
    if (data->value != -1) {
      printf("read: %d\n", data->value);
    }

    sleep(1);  // short delay
  }

  // change server state.
  data->stop = 1;

  shmdt(data);                    // Detach from memory segment.
  shmctl(shmid, IPC_RMID, NULL);  // Dispose memory segment.
  printf("Server is finished.\n");

  return 0;
}
