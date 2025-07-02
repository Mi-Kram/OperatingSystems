#include "general.h"

// IPC id.
int semid = -1, msgid = -1;

// pids of other programs.
pid_t pid_petrov = -1;

// finishes the program.
void handle_sigint(int) {
  kill(pid_petrov, SIGINT);
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

  struct sembuf sem_necheporuk = {SEM_NECHEPORUK, -1, 0};
  struct sembuf sem_ivanov = {SEM_IVANOV, 1, 0};

  while (1) {
    // wait for the petrov action to start.
    if (semop(semid, &sem_necheporuk, 1) == -1) break;

    // emulate the time of necheporuk action.
    sleep(rand() % 3 + 1);

    // read name of stolen item.
    struct msgbuf received;
    int len = msgrcv(msgid, &received, sizeof(received.mtext) - 1, 1, 0);
    if (len == -1) break;
    received.mtext[len] = 0;

    // generate a cost.
    int price = rand() % 1000 + 100;

    printf("Necheporuk: valued a %s at %d. Total cost: %d\n", received.mtext, price, total += price);

    // pass the baton to necheporuk process.
    if (semop(semid, &sem_ivanov, 1) == -1) break;
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
  semid = semget(key, SEM_COUNT, 0666);
  if (semid == -1) {
    printf("semget failed\n");
    return -1;
  }

  // open message queue.
  msgid = msgget(key, 0666);
  if (msgid == -1) {
    printf("msgget failed");
    return -1;
  }

  // get pid of previous program and send a current pid.
  if (pid_exchange() == -1) return -1;
  
  return 0;
}


int pid_exchange() {
  struct sembuf sem_necheporuk = {SEM_NECHEPORUK, -1, 0};
  struct sembuf sem_ivanov = {SEM_IVANOV, 1, 0};

  if (semop(semid, &sem_necheporuk, 1) == -1) {
    printf("pid_exchange: semop failed\n");
    return -1;
  }

  // read pid.
  struct msgbuf received;
  int len = msgrcv(msgid, &received, sizeof(received.mtext) - 1, 1, 0);
  if (len == -1) {
    printf("pid_exchange: msgrcv failed\n");
    return -1;
  }
  received.mtext[len] = 0;

  // parse pid.
  if (sscanf(received.mtext, "%d", &pid_petrov) <= 0) {
    printf("pid_exchange: sscanf pid failed\n");
    return -1;
  }

  // write current pid.
  struct msgbuf message;
  message.mtype = 1;
  len = sprintf(message.mtext, "%d", getpid());
  message.mtext[len] = 0;
  if (msgsnd(msgid, &message, len, 0) == -1) {
    printf("pid_exchange: msgsnd failed\n");
    return -1;
  }

  if (semop(semid, &sem_ivanov, 1) == -1) {
    printf("pid_exchange: semop failed\n");
    return -1;
  }

  return 0;
}

