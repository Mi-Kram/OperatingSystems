// #include <signal.h>
#include <unistd.h>
// #include <stdio.h>

int main(){
  (void)fork();
  while(1);
  return 0;
}
