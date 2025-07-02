#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Value that is neither fibonacci nor factorial.
const unsigned long long OVERFLOW = 4;

unsigned long long fibo(int arg);
unsigned long long fact(int arg);

int main(int argc, char **argv) {
  // Check amount of arguments.
  if (argc != 2) {
    printf("Set one parameter - fibonacci and factorial argument\n");
    return 0;
  }

  // Try parse a numeric argument.
  int arg;
  if (!sscanf(argv[1], "%d", &arg) || arg < 0) {
    printf("Set one parameter - fibonacci and factorial argument\n");
    return 0;
  }

  pid_t root = getpid();  // pid of main process.
  fork();
  unsigned long long res;

  // Print information about process.
  if (root == getpid()) printf("Parent: ");
  else printf("Child:  ");
  printf("pid = %d | parent pid = %d\n", getpid(), getppid());

  // Make calculations.
  if (root == getpid()) {  // parent
    res = fibo(arg);
    printf("fibonacci(%d) = ", arg);
  } else {                 // child
    res = fact(arg);
    printf("factorial(%d) = ", arg);
  }

  // Print the result.
  if (res == OVERFLOW) printf("OVERFLOW\n");
  else printf("%lld\n", res);

  // Print information about current directory.
  if (root == getpid()) {
    printf("\nInformation about the contents of the current directory:\n");
    system("pwd & ls -alh");
  }
  
  return 0;
}

unsigned long long fibo(int arg) {
  if (arg <= 0) return 0;
  unsigned long long prev = 0, cur = 1, next;

  while (--arg) {
    next = cur + prev;
    if (next < cur) return OVERFLOW;
  
    prev = cur;
    cur = next;
  }

  return cur;
}

unsigned long long fact(int arg) {
  if (arg < 0) return 0;

  unsigned long long res = 1, next;
  while (arg) {
    next = res * arg;
    if (next / arg != res) return OVERFLOW;

    res = next;
    --arg;
  }

  return res;
}

