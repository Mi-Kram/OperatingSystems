#include "general.h"

// amount of military things to stole.
int military_things;

// IPC id.
int semid = -1, msgid = -1;

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

  struct sembuf sem_ivanov = {SEM_IVANOV, -1, 0};
  struct sembuf sem_petrov = {SEM_PETROV, 1, 0};

  while (++i <= military_things) {
    // wait for the ivanov action to start.
    if (semop(semid, &sem_ivanov, 1) == -1) break;

    // emulate the time of ivanov action.
    sleep(rand() % 3 + 1);
    if (!military_things) break;

    // store name of stolen item.
    struct msgbuf message;
    message.mtype = 1;
    int len = sprintf(message.mtext, "Box #%d", i);
    message.mtext[len] = 0;
    if (msgsnd(msgid, &message, len, 0) == -1) break;

    printf("Ivanov: took out a %s\n", message.mtext);

    // pass the baton to petrov process.
    if (semop(semid, &sem_petrov, 1) == -1) break;
  }

  if (i > military_things) {
    semop(semid, &sem_ivanov, 1);
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
  key_t key = get_ipc_key();
  if (key == -1) {
    printf("get_ipc_key failed\n");
    return -1;
  }

  // id of semaphores.
  semid = semget(key, SEM_COUNT, IPC_CREAT | 0666);
  if (semid == -1) {
    printf("semget failed\n");
    return -1;
  }

  union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
  };

  // init semaphores.
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

  // open message queue.
  msgid = msgget(key, IPC_CREAT | 0666);
  if (msgid == -1) {
    printf("msgget failed");
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
  struct sembuf sem_ivanov = {SEM_IVANOV, -1, 0};
  struct sembuf sem_ivanov_free = {SEM_IVANOV, 1, 0};
  struct sembuf sem_petrov = {SEM_PETROV, 1, 0};

  if (semop(semid, &sem_ivanov, 1) == -1) {
    printf("pid_exchange: semop failed\n");
    return -1;
  }

  // send current pid.
  struct msgbuf message;
  message.mtype = 1;
  int len = sprintf(message.mtext, "%d", getpid());
  message.mtext[len] = 0;
  if (msgsnd(msgid, &message, len, 0) == -1) {
    printf("pid_exchange: msgsnd failed\n");
    return -1;
  }

  if (semop(semid, &sem_petrov, 1) == -1) {
    printf("pid_exchange: semop failed\n");
    return -1;
  }

  if (semop(semid, &sem_ivanov, 1) == -1) {
    printf("pid_exchange: semop failed\n");
    return -1;
  }

  // read pid.
  struct msgbuf received;

  len = msgrcv(msgid, &received, sizeof(received.mtext) - 1, 1, 0);
  if (len == -1) {
    printf("pid_exchange: msgrcv failed\n");
    return -1;
  }
  received.mtext[len] = 0;

  // parse pid.
  if (sscanf(received.mtext, "%d", &pid_necheporuk) <= 0) {
    printf("pid_exchange: sscanf pid failed\n");
    return -1;
  }

  if (semop(semid, &sem_ivanov_free, 1) == -1) {
    printf("pid_exchange: semop failed\n");
    return -1;
  }

  return 0;
}

void dispose() {
  // close fifo.
  msgctl(msgid, IPC_RMID, 0);

  // close semaphores.
  semctl(semid, 0, IPC_RMID, 0);
}
