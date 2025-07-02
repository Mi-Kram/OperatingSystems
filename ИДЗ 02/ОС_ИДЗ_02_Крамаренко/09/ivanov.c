#include "general.h"

// amount of military things to stole.
int military_things;

// file descriptors to share data.
int fifo_fd = -1;

// program semaphores.
sem_t *sem_ivanov = 0, *sem_petrov = 0, *sem_necheporuk = 0;

// pids of other programs.
pid_t pid_necheporuk = -1;

// releases program resources
void dispose();

// finishes the program.
void handle_sigint(int) {
  kill(pid_necheporuk, SIGINT);
  pid_necheporuk = -1;
  dispose();
  exit(0);
}

// checks command line arguments
int check_arguments(int argc, char **argv);

// initializes program variables.
int initialize();

// get pid of previous program and send a current pid.
int pid_exchange();

int main(int argc, char **argv) {
  if (check_arguments(argc, argv) == -1) return 0;

  // intercept SIGINT signal.
  signal(SIGINT, handle_sigint);
  if (initialize() == -1) return 0;

  printf("Start operation:\n");
  int i = 0;  // stolen things.

  while (++i <= military_things) {
    // wait for the ivanov action to start.
    if (sem_wait(sem_ivanov) == -1) break;

    // emulate the time of ivanov action.
    sleep(rand() % 3 + 1);
    if (!military_things) break;

    // store name of stolen item.
    char buf[64];
    int len = sprintf(buf, "Box #%d", i);
    buf[len] = 0;
    if (write(fifo_fd, buf, len) == -1) break;

    printf("Ivanov: took out a %s\n", buf);

    // pass the baton to petrov process.
    if (sem_post(sem_petrov) == -1) break;
  }

  if (i > military_things) {
    sem_wait(sem_ivanov);
    printf("Ivanov: everything is stolen\n");
  }

  kill(pid_necheporuk, SIGINT);
  pid_necheporuk = -1;
  dispose();
  return 0;
}

int check_arguments(int argc, char **argv) {
  if (argc == 1) {
    military_things = 10;
    return 0;
  }

  if (argc != 2) {
    printf("Usage: %s <amount>\n", argv[0]);
    return -1;
  }

  int cnt = sscanf(argv[1], "%d", &military_things);

  if (cnt <= 0) {
    printf("Usage: %s <amount>\n", argv[0]);
    return -1;
  }

  if (military_things <= 0) {
    printf("Amount should be greater than zero\n");
    return -1;
  }

  return 0;
}

int initialize() {
  unlink(FIFO);
  sem_unlink(SEM_IVANOV);
  sem_unlink(SEM_PETROV);
  sem_unlink(SEM_NECHEPORUK);

  // Create FIFO.
  if (mkfifo(FIFO, 0666) == -1) {
    printf("Can not create a FIFO_IVANOV_PETROV\n");
    dispose();
    return -1;
  }

  // open semaphore for ivanov.
  sem_ivanov = sem_open(SEM_IVANOV, O_CREAT, 0666, 1);
  if (!sem_ivanov || sem_ivanov == SEM_FAILED) {
    printf("Can not initialize ivanov semaphore\n");
    dispose();
    return -1;
  }

  // open semaphore for petrov.
  sem_petrov = sem_open(SEM_PETROV, O_CREAT, 0666, 0);
  if (!sem_petrov || sem_petrov == SEM_FAILED) {
    printf("Can not initialize petrov semaphore\n");
    dispose();
    return -1;
  }

  // open semaphore for necheporuk.
  sem_necheporuk = sem_open(SEM_NECHEPORUK, O_CREAT, 0666, 0);
  if (!sem_necheporuk || sem_necheporuk == SEM_FAILED) {
    printf("Can not initialize necheporuk semaphore\n");
    dispose();
    return -1;
  }

  // open fifo to write the data.
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
  if (sem_wait(sem_ivanov) == -1) {
    printf("pid_exchange: sem_wait failed\n");
    return -1;
  }

  // send current pid.
  char buf[24];
  int len = sprintf(buf, "%d", getpid());
  if (write(fifo_fd, buf, len) == -1) {
    printf("pid_exchange: write pid failed\n");
    return -1;
  }

  if (sem_post(sem_petrov) == -1) {
    printf("pid_exchange: sem_post failed\n");
    return -1;
  }

  if (sem_wait(sem_ivanov) == -1) {
    printf("pid_exchange: sem_wait failed\n");
    return -1;
  }

  // read pid.
  len = read(fifo_fd, buf, len);
  if (len == -1) {
    printf("pid_exchange: read pid failed\n");
    return -1;
  }

  // parse pid.
  if (sscanf(buf, "%d", &pid_necheporuk) <= 0) {
    printf("pid_exchange: sscanf pid failed\n");
    return -1;
  }

  if (sem_post(sem_ivanov) == -1) {
    printf("pid_exchange: sem_post failed\n");
    return -1;
  }

  return 0;
}

void dispose() {
  // close fifo.
  close(fifo_fd);

  // unlink fifo.
  unlink(FIFO);

  // close semaphores.
  sem_close(sem_ivanov);
  sem_close(sem_petrov);
  sem_close(sem_necheporuk);

  // unlink semaphores.
  sem_unlink(SEM_IVANOV);
  sem_unlink(SEM_PETROV);
  sem_unlink(SEM_NECHEPORUK);
}
