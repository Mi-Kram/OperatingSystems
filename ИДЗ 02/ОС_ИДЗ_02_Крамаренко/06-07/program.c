#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Name of shared memory.
#define SHM_NAME "/military_object"

// Shared memory object type.
typedef struct {
  char item[64];
  int value;

  int running;

  // program semaphores.
  sem_t sem_ivanov;
  sem_t sem_petrov;
  sem_t sem_necheporuk;
} Item;

int shm_fd = -1;   // file descriptor of shared memory.
Item *shared = 0;  // pointer of shared memory.

// amount of military things to stole
int military_things;

// finishes the program.
void handle_sigint(int) {
  if (shared != 0) shared->running = 0;
}

// checks command line arguments.
int check_arguments(int argc, char **argv);

// initializes program variables.
int initialize();

// releases program resources.
void dispose();

// starts process for ivanov.
// returns -1 if process is not created.
int start_ivanov_process(sem_t *sem_ivanov, sem_t *sem_petrov, Item *shared);

// starts process for petrov.
// returns -1 if process is not created.
int start_petrov_process(sem_t *sem_petrov, sem_t *sem_necheporuk, Item *shared);

// starts process for necheporuk.
// returns -1 if process is not created.
int start_necheporuk_process(sem_t *sem_necheporuk, sem_t *sem_ivanov, Item *shared);

int main(int argc, char **argv) {
  if (check_arguments(argc, argv) == -1) return 0;

  // intercept SIGINT signal.
  signal(SIGINT, handle_sigint);

  // initialize program.
  if (initialize() == -1) return 0;

  printf("Start operation:\n\n");
  shared->running = 1;

  // start processes.
  if (start_ivanov_process(&shared->sem_ivanov, &shared->sem_petrov, shared) == -1) {
    dispose();
    return 0;
  }
  if (start_petrov_process(&shared->sem_petrov, &shared->sem_necheporuk, shared) == -1) {
    dispose();
    return 0;
  }
  if (start_necheporuk_process(&shared->sem_necheporuk, &shared->sem_ivanov, shared) == -1) {
    dispose();
    return 0;
  }

  // the parent waits for child processes to finish.
  wait(NULL);
  wait(NULL);
  wait(NULL);

  // realeas the resources.
  printf("Operation is finished.\n");
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
  // open or create shared memory.
  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    printf("Can not open shared memory\n");
    return -1;
  }

  // set size of shared memory object.
  if (ftruncate(shm_fd, sizeof(Item)) == -1) {
    printf("Can not truncate shared memory\n");
    close(shm_fd);
    return -1;
  }

  // map shared memory object.
  shared = mmap(0, sizeof(Item), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared == (void *)-1) {
    printf("Can not mmap shared memory\n");
    close(shm_fd);
    return -1;
  }

  // open semaphore for ivanov.
  if (sem_init(&shared->sem_ivanov, 1, 1) == -1) {
    printf("Can not initialize ivanov semaphore\n");
    munmap(shared, sizeof(Item));
    close(shm_fd);
    return -1;
  }

  // open semaphore for petrov.
  if (sem_init(&shared->sem_petrov, 1, 0) == -1) {
    printf("Can not initialize petrov semaphore\n");
    munmap(shared, sizeof(Item));
    close(shm_fd);
    return -1;
  }

  // open semaphore for necheporuk.
  if (sem_init(&shared->sem_necheporuk, 1, 0) == -1) {
    printf("Can not initialize necheporuk semaphore\n");
    munmap(shared, sizeof(Item));
    close(shm_fd);
    return -1;
  }

  return 0;
}

void dispose() {
  // unmap and close shared memory.
  if (shared != 0 && shared != (void *)-1) {
    shared->running = 0;
    munmap(shared, sizeof(Item));
  }
  if (shm_fd != -1) close(shm_fd);

  // unlink shared memory.
  shm_unlink(SHM_NAME);

  // close semaphores.
  sem_destroy(&shared->sem_ivanov);
  sem_destroy(&shared->sem_petrov);
  sem_destroy(&shared->sem_necheporuk);
}

int start_ivanov_process(sem_t *sem_ivanov, sem_t *sem_petrov, Item *shared) {
  // fork the main process.
  pid_t pid = fork();
  if (pid == -1) {
    printf("Can not fork ivanov process\n");
    return -1;
  }

  // parent process.
  if (pid != 0) return 0;

  srand(time(0));
  int i = 0;  // stolen things.

  while (shared->running && i < military_things) {
    // wait for the ivanov action to start.
    sem_wait(sem_ivanov);
    if (!shared->running) break;

    // emulate the time of ivanov action.
    sleep(rand() % 3 + 1);
    if (!shared->running) break;

    // store data.
    sprintf(shared->item, "Box #%d", ++i);
    shared->value = rand() % 1000 + 100;
    printf("Ivanov: took out a %s\n", shared->item);

    // pass the baton to petrov process.
    sem_post(sem_petrov);
  }

  // if ivanov action is finished due to program finishing.
  if (!shared->running) {
    sem_post(sem_petrov);
    exit(0);
  }

  // wait for the ivanov action to start.
  sem_wait(sem_ivanov);

  // stop the program.
  shared->running = 0;
  printf("Ivanov: everything is stolen\n\n");

  // pass the baton to petrov process.
  sem_post(sem_petrov);
  exit(0);
}

int start_petrov_process(sem_t *sem_petrov, sem_t *sem_necheporuk, Item *shared) {
  // fork the main process.
  pid_t pid = fork();
  if (pid == -1) {
    printf("Can not fork petrov process\n");
    return -1;
  }

  // parent process.
  if (pid != 0) return 0;

  while (shared->running) {
    // wait for the petrov action to start.
    sem_wait(sem_petrov);
    if (!shared->running) break;

    // emulate the time of petrov action.
    sleep(rand() % 3 + 1);
    if (!shared->running) break;

    printf("Petrov: uploaded a %s\n", shared->item);

    // pass the baton to necheporuk process.
    sem_post(sem_necheporuk);
  }

  // pass the baton to necheporuk process.
  sem_post(sem_necheporuk);
  exit(0);
}

int start_necheporuk_process(sem_t *sem_necheporuk, sem_t *sem_ivanov, Item *shared) {
  // fork the main process.
  pid_t pid = fork();
  if (pid == -1) {
    printf("Can not fork necheporuk process\n");
    return -1;
  }

  // parent process.
  if (pid != 0) return 0;

  int total = 0;
  while (shared->running) {
    // wait for the necheporuk action to start.
    sem_wait(sem_necheporuk);
    if (!shared->running) break;

    // emulate the time of necheporuk action.
    sleep(rand() % 3 + 1);
    if (!shared->running) break;

    printf("Necheporuk: valued a %s at %d. Total cost: %d\n\n", shared->item, shared->value, total += shared->value);

    // pass the baton to ivanov process.
    sem_post(sem_ivanov);
  }

  // pass the baton to ivanov process.
  sem_post(sem_ivanov);
  exit(0);
}
