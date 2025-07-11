#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void my_handler(int nsig){
   printf("Received signal %d, CTRL-C pressed\n", nsig);
}

int main(){
  (void)signal(SIGINT, my_handler);
  while(1);
  return 0;
}
