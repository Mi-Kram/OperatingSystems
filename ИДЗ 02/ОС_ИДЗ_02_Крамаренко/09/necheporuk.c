#include "general.h"

// file descriptors to share data.
int fifo_fd = -1;

// program semaphores.
sem_t *sem_ivanov = 0, *sem_necheporuk = 0;

// pids of other programs.
pid_t pid_petrov = -1;

// releases program resources
void dispose();

// finishes the program.
void handle_sigint(int) {
  kill(pid_petrov, SIGINT);
  pid_petrov = -1;
  dispose();
  exit(0);
}

// initializes program variables.
int initialize();

// get pid of previous program and send a current pid.
int pid_exchange();

int main() {
  // intercept SIGINT signal.
  signal(SIGINT, handle_sigint);

  if (initialize() == -1) return 0;
  printf("Start operation:\n");
  int total = 0;

  srand(time(0));

  while (1) {
    // wait for the petrov action to start.
    if (sem_wait(sem_necheporuk) == -1) break;

    // emulate the time of necheporuk action.
    sleep(rand() % 3 + 1);

    // read name of stolen item.
    char buf[64];
    int len = read(fifo_fd, buf, 63);
    if (len == -1) break;
    buf[len] = 0;

    // generate a cost.
    int price = rand() % 1000 + 100;

    printf("Necheporuk: valued a %s at %d. Total cost: %d\n", buf, price, total += price);

    // pass the baton to necheporuk process.
    if (sem_post(sem_ivanov) == -1) break;
  }

  dispose();
  return 0;
}

void dispose() {
  // close fifo.
  close(fifo_fd);

  // close semaphores.
  sem_close(sem_ivanov);
  sem_close(sem_necheporuk);
}

int initialize() {
  // open semaphore for ivanov.
  sem_ivanov = sem_open(SEM_IVANOV, 0);
  if (!sem_ivanov || sem_ivanov == SEM_FAILED) {
    printf("Can not open ivanov semaphore\n");
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
  if (sem_wait(sem_necheporuk) == -1) {
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

  // parse pid.
  if (sscanf(buf, "%d", &pid_petrov) <= 0) {
    printf("pid_exchange: sscanf pid failed\n");
    return -1;
  }

  // write current pid.
  len = sprintf(buf, "%d", getpid());
  if (write(fifo_fd, buf, len) == -1) {
    printf("pid_exchange: write pid failed\n");
    return -1;
  }

  if (sem_post(sem_ivanov) == -1) {
    printf("pid_exchange: sem_post failed\n");
    return -1;
  }

  return 0;
}

