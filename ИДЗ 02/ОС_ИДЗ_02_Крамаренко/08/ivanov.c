#include "general.h"

// amount of military things to stole
int military_things;

// shared memory object
SharedData *shared = 0;
int shmid;
int semid;

// finishes the program.
void handle_sigint(int) {
  if (shared != 0) shared->running = 0;
}

// checks command line arguments
int check_arguments(int argc, char **argv);

// initializes program variables.
int initialize();

// releases program resources
void dispose();

int main(int argc, char **argv) {
  if (check_arguments(argc, argv) == -1) return 0;

  // intercept SIGINT signal.
  signal(SIGINT, handle_sigint);

  if (initialize() == -1) return 0;
  
  struct sembuf sem_ivanov = {SEM_IVANOV, -1, 0};
  struct sembuf sem_petrov = {SEM_PETROV, 1, 0};

  srand(getpid());
  printf("Start operation:\n");
  shared->running = 1;
  int i = 0;  // stolen things.

  while (shared->running && i < military_things) {
    // wait for the ivanov action to start.
    semop(semid, &sem_ivanov, 1);
    if (!shared->running) break;

    // emulate the time of ivanov action.
    sleep(rand() % 3 + 1);
    if (!shared->running) break;

    // store data.
    sprintf(shared->item, "Box #%d", ++i);
    shared->value = rand() % 1000 + 100;
    printf("Ivanov: took out a %s\n", shared->item);

    // pass the baton to petrov process.
    semop(semid, &sem_petrov, 1);
  }

  // if ivanov action is finished due to program finishing.
  if (!shared->running) {
    semop(semid, &sem_petrov, 1);
    dispose();
    return 0;
  }

  // wait for the ivanov action to start.
  semop(semid, &sem_ivanov, 1);

  // stop the program.
  shared->running = 0;
  printf("Ivanov: everything is stolen\n");

  // pass the baton to petrov process.
  semop(semid, &sem_petrov, 1);
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
  key_t key = get_ipc_key();
  if (key == -1) {
    printf("get_ipc_key failed\n");
    return -1;
  }

  // shared memory.
  shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
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
  semid = semget(key, SEM_COUNT, IPC_CREAT | 0666);
  if (semid == -1) {
    printf("semget failed\n");
    dispose();
    return -1;
  }

  union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
  };
  
  union semun arg;
  arg.val = 1;
  if (semctl(semid, SEM_IVANOV, SETVAL, arg) == -1) {
    printf("semctl failed for ivanov");
    dispose();
    return -1;
  }
  arg.val = 0;

  if (semctl(semid, SEM_PETROV, SETVAL, arg) == -1) {
    printf("semctl failed for petrov");
    dispose();
    return -1;
  }
  if (semctl(semid, SEM_NECHEPORUK, SETVAL, arg) == -1) {
    printf("semctl failed for necheporuk");
    dispose();
    return -1;
  }

  return 0;
}

void dispose() {
  shmdt(shared);
  shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);

  semctl(semid, 0, IPC_RMID, 0);
}
