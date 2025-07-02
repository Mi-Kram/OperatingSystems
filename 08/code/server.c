#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "general.h"

// flag: state of grogram: 1 - running, 0 - stopped.
volatile sig_atomic_t running = 1;

// handle program aborting.
void stop_running(int) { running = 0; }

int main(int argc, char* argv[]) {
  // create new shared memory.
  int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0666);
  if (shm_fd == -1) {
    perror("error: shm_open returns -1");
    return 0;
  }

  // truncate a file to a specified length.
  if (ftruncate(shm_fd, sizeof(data_t)) == -1) {
    perror("error: ftruncate returns -1");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 0;
  }

  // get an access to the memory.
  data_t* data = (data_t*)mmap(NULL, sizeof(data_t), PROT_READ | PROT_WRITE,
                               MAP_SHARED, shm_fd, 0);
  if (data == MAP_FAILED) {
    perror("error: mmap failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);
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
      data->value = -1;
    }

    sleep(1);  // short delay
  }

  // change server state.
  data->stop = 1;

  // dispose shared memory resources.
  munmap(data, sizeof(data_t));
  close(shm_fd);
  shm_unlink(SHM_NAME);

  printf("Server is finished.\n");

  return 0;
}
