#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


volatile sig_atomic_t num = 0, pos = 0, isSenderReady = 0;
pid_t sender;

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

int main() {
  signal(SIGUSR1, onReceiving);
  signal(SIGUSR2, onReceiving);

  printf("Receiver pid: %d\n", getpid());

  // Input a sender pid.
  printf("Enter a sender pid: ");
  if (scanf("%d", &sender) != 1) {
    printf("Wrong input\n");
    return 0;
  }

  // Check correctness of receiver pid.
  // At least it should be another process and it should exist.
  if (sender == getpid() || getpgid(sender) == -1) {
    printf("Wrong pid\n");
    return 0;
  }

  // Send first signal: receiver is ready to receive a number.
  kill(sender, SIGUSR1);

  // Wait for the sender to also send its ready state.
  while (!isSenderReady) pause();

  // Wait until all bits are set.
  int bits = CHAR_BIT * sizeof(int), bit;
  while (pos != bits) pause();

  printf("Number received successfully: %d\n", num);
  return 0;
}
