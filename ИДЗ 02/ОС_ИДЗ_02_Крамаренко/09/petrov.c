#include "general.h"

// file descriptors to share data.
int fifo_fd = -1;

// program semaphores.
sem_t *sem_petrov = 0, *sem_necheporuk = 0;

// pids of other programs.
pid_t pid_ivanov = -1;

// releases program resources
void dispose();

// get pid of previous program and send a current pid.
int pid_exchange();

// finishes the program.
void handle_sigint(int) {
  kill(pid_ivanov, SIGINT);
  pid_ivanov = -1;
  dispose();
  exit(0);
}

// initializes program variables.
int initialize();

int main() {
  // intercept SIGINT signal.
  signal(SIGINT, handle_sigint);

  if (initialize() == -1) return 0;
  printf("Start operation:\n");

  while (1) {
    // wait for the petrov action to start.
    if (sem_wait(sem_petrov) == -1) break;

    // emulate the time of petrov action.
    sleep(rand() % 3 + 1);

    // read name of stolen item.
    char buf[64];
    int len = read(fifo_fd, buf, 63);
    if (len == -1) break;
    buf[len] = 0;

    printf("Petrov: uploaded a %s\n", buf);

    // store name of stolen item.
    if (write(fifo_fd, buf, len) == -1) break;

    // pass the baton to necheporuk process.
    if (sem_post(sem_necheporuk) == -1) break;
  }

  dispose();
  return 0;
}

void dispose() {
  // close fifo.
  close(fifo_fd);

  // close semaphores.
  sem_close(sem_petrov);
  sem_close(sem_necheporuk);
}

int initialize() {
  // open semaphore for petrov.
  sem_petrov = sem_open(SEM_PETROV, 0);
  if (!sem_petrov || sem_petrov == SEM_FAILED) {
    printf("Can not open petrov semaphore\n");
    dispose();
    return -1;
  }

  // open semaphore for necheporuk.
  sem_necheporuk = sem_open(SEM_NECHEPORUK, 0);
  if (!sem_necheporuk || sem_necheporuk == SEM_FAILED) {
    printf("Can not open necheporuk semaphore\n");
    dispose();
    return -1;
  }

  // open fifo to read the data.
  fifo_fd = open(FIFO, O_RDWR);
  if (fifo_fd == -1) {
    printf("Can not open a FIFO\n");
    dispose();
    return -1;
  }

  // get pid of previous program and send a current pid.
  if (pid_exchange() == -1) {
    dispose();
    return -1;
  }
  
  return 0;
}

int pid_exchange() {
  if (sem_wait(sem_petrov) == -1) {
    printf("pid_exchange: sem_wait failed\n");
    return -1;
  }

  // read pid.
  char buf[24];
  int len = read(fifo_fd, buf, len);
  if (len == -1) {
    printf("pid_exchange: read pid failed\n");
    return -1;
  }

  // pserse pid.
  if (sscanf(buf, "%d", &pid_ivanov) <= 0) {
    printf("pid_exchange: sscanf pid failed\n");
    return -1;
  }

  // write current pid.
  len = sprintf(buf, "%d", getpid());
  if (write(fifo_fd, buf, len) == -1) {
    printf("pid_exchange: write pid failed\n");
    return -1;
  }

  if (sem_post(sem_necheporuk) == -1) {
    printf("pid_exchange: sem_post failed\n");
    return -1;
  }

  return 0;
}

