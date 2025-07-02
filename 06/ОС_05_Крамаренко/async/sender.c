#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


volatile sig_atomic_t isReceived = 0;
pid_t childPid;

void exitHandler(int nsig) {
  kill(childPid, SIGKILL);
  exit(0);
}

void onReceived(int nsig) {
  isReceived = 1;
}

void runAsync();

int main() {
  signal(SIGINT, exitHandler); signal(SIGALRM, exitHandler);
  signal(SIGABRT, exitHandler); signal(SIGKILL, exitHandler);
  signal(SIGQUIT, exitHandler); // There are 31 signals 😮.

  if ((childPid = fork()) == 0) {
    runAsync();
    exit(0);
  }

  wait(NULL);
  return 0;
}

void runAsync() {
  void (*prevHandler)(int) = signal(SIGUSR1, onReceived);

  printf("Sender pid: %d\n", getpid());

  // Input a receiver pid.
  pid_t receiver;
  printf("Enter a receiver pid: ");
  if (scanf("%d", &receiver) != 1) {
    printf("Wrong input\n");
    signal(SIGUSR1, prevHandler);
    return;
  }

  // Check correctness of receiver pid.
  // At least it should be another process and it should exist.
  if (receiver == getpid() || getpgid(receiver) == -1) {
    printf("Wrong pid\n");
    signal(SIGUSR1, prevHandler);
    return;
  }

  // Send first signal: sender is ready to send a number.
  kill(receiver, SIGUSR1);

  // Wait for the receiver to also send its ready state.
  while (!isReceived) pause();

  // Input number to transfer.
  int num;
  printf("Enter the number to transfer: ");
  if (scanf("%d", &num) != 1) {
    printf("Wrong input\n");
    signal(SIGUSR1, prevHandler);
    return;
  }

  // Start sending the number from right to left.
  int bits = CHAR_BIT * sizeof(int), bit;
  for (int i = 0; i < bits; ++i) {
    bit = (num >> i) & 1; // Bit to send.

    // Reset state. Send signal. And wait for a confirmation.
    isReceived = 0;
    kill(receiver, bit ? SIGUSR1 : SIGUSR2);
    while (!isReceived) pause();
  }

  printf("Number is transferred successfully.\n");
  signal(SIGUSR1, prevHandler);
}
