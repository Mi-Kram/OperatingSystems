#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[], char *env[]) {
  // execlp("/bin/cat", "/bin/cat", argv[1], NULL);
  execle("/bin/cat", "/bin/cat", argv[1], NULL, env);
  // Сюда попадаем только при возникновении ошибки
  printf("Error on program: %s %s\n", argv[0], argv[1]);
  return -1;  // Информация об ошибке
}
