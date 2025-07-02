#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


volatile sig_atomic_t isReceived = 0;

void onReceived(int nsig) {
  isReceived = 1;
}

int main() {
  signal(SIGUSR1, onReceived);

  printf("Sender pid: %d\n", getpid());

  // Input a receiver pid.
  pid_t receiver;
  printf("Enter a receiver pid: ");
  if (scanf("%d", &receiver) != 1) {
    printf("Wrong input\n");
    return 0;
  }

  // Check correctness of receiver pid.
  // At least it should be another process and it should exist.
  if (receiver == getpid() || getpgid(receiver) == -1) {
    printf("Wrong pid\n");
    return 0;
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
    return 0;
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
  return 0;
}
