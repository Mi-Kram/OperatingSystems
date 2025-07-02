#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


volatile sig_atomic_t num = 0, pos = 0, isSenderReady = 0;
pid_t sender, childPid;

void exitHandler(int nsig) {
  kill(childPid, SIGKILL);
  exit(0);
}

void onReceiving(int nsig) {
  // First signal means that sender is ready to send the number.
  if (!isSenderReady) {
    isSenderReady = 1;
    return;
  }

  // Determine the bit. Insert the bit. Send the confirmation.
  int bit = nsig == SIGUSR1 ? 1 : 0;
  num |= (bit << pos++);
  kill(sender, SIGUSR1);
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
  void (*prevHandler1)(int) = signal(SIGUSR1, onReceiving);
  void (*prevHandler2)(int) = signal(SIGUSR2, onReceiving);

  printf("Receiver pid: %d\n", getpid());

  // Input a sender pid.
  printf("Enter a sender pid: ");
  if (scanf("%d", &sender) != 1) {
    printf("Wrong input\n");
    signal(SIGUSR1, prevHandler1);
    signal(SIGUSR2, prevHandler2);
    return;
  }

  // Check correctness of receiver pid.
  // At least it should be another process and it should exist.
  if (sender == getpid() || getpgid(sender) == -1) {
    printf("Wrong pid\n");
    signal(SIGUSR1, prevHandler1);
    signal(SIGUSR2, prevHandler2);
    return;
  }

  // Send first signal: receiver is ready to receive a number.
  kill(sender, SIGUSR1);

  // Wait for the sender to also send its ready state.
  while (!isSenderReady) pause();

  // Wait until all bits are set.
  int bits = CHAR_BIT * sizeof(int), bit;
  while (pos != bits) pause();

  printf("Number received successfully: %d\n", num);
  signal(SIGUSR1, prevHandler1);
  signal(SIGUSR2, prevHandler2);
}
