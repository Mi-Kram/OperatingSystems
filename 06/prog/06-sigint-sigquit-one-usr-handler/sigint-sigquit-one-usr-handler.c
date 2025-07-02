#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void my_sigint_handler(int nsig){
   if (nsig == SIGINT) {
      printf("Received signal %d, CTRL+C pressed\n", nsig);
   } else if (nsig == SIGQUIT) {
      printf("Received signal %d, CTRL+4 pressed\n", nsig);
   }
}

int main(){
  (void)signal(SIGINT, my_sigint_handler);
  (void)signal(SIGQUIT, my_sigint_handler);
  while(1);
  return 0;
}
