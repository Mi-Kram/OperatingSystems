#include "general.h"

// shared memory object
SharedData *shared = 0;
int shmid;
int semid;

// finishes the program.
void handle_sigint(int) {
  if (shared != 0) shared->running = 0;
}

// initializes program variables.
int initialize();

// releases program resources
void dispose();

int main() {
  // intercept SIGINT signal.
  signal(SIGINT, handle_sigint);

  if (initialize() == -1) return 0;

  struct sembuf sem_necheporuk = {SEM_NECHEPORUK, -1, 0};
  struct sembuf sem_ivanov = {SEM_IVANOV, 1, 0};

  int total = 0;

  if (shared->running) printf("Start operation:\n");
  else printf("Program is not ready\n");

  while (shared->running) {
    // wait for the petrov action to start.
    semop(semid, &sem_necheporuk, 1);
    if (!shared->running) break;

    // emulate the time of necheporuk action.
    sleep(rand() % 3 + 1);
    if (!shared->running) break;

    printf("Necheporuk: valued a %s at %d. Total cost: %d\n", shared->item, shared->value, total += shared->value);

    // pass the baton to necheporuk process.
    semop(semid, &sem_ivanov, 1);
  }

  // pass the baton to necheporuk process.
  semop(semid, &sem_ivanov, 1);

  dispose();
  return 0;
}

void dispose() {
  shmdt(shared);
}

int initialize() {
  key_t key = get_ipc_key();
  if (key == -1) {
    printf("get_ipc_key failed\n");
    return -1;
  }

  // shared memory.
  shmid = shmget(key, sizeof(SharedData), 0666);
  if (shmid == -1) {
    printf("shmget failed\n");
    return -1;
  }

  shared = shmat(shmid, NULL, 0);
  if (shared == (void*)-1) {
    printf("shmat failed\n");
    dispose();
    return -1;
  }

  // id of semaphores.
  semid = semget(key, SEM_COUNT, 0666);
  if (semid == -1) {
    printf("semget failed\n");
    dispose();
    return -1;
  }

  return 0;
}

