#include <signal.h>
#include <stdio.h>

int i=0;
void (*p)(int);

void my_handler(int nsig){
   printf("\nAttempt %d. Receive signal %d, CTRL-C pressed\n",
          ++i, nsig);
   if(i == 5)
       (void)signal(SIGINT, p);
}

int main(void){
    p = signal(SIGINT, my_handler);
    while(1);
    return 0;
}
