/* Программа sem-posix-post.c
 * для иллюстрации работы с неименованным POSIX-семафором.
 * Эта программа получает доступ к одному системному семафору
 * и увеличивает его на 1
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  const char* shar_object = "/posix-shar-object";
  int shmid;            // дескриптор разделяемой памяти

  // Открыть объект памяти
  if ( (shmid = shm_open(shar_object, O_RDWR, 0666)) == -1 ) {
    printf("Opening error\n");
    perror("shm_open");
    return 1;
  } else {
    printf("Object is open: name = %s, id = 0x%x\n", shar_object, shmid);
  }

  // Получение доступа к разделяемой памяти
  sem_t *sem_p = mmap(0, sizeof (sem_t), PROT_WRITE|PROT_READ, MAP_SHARED, shmid, 0);

  // Увеличение значения семафора на 1
  if(sem_post(sem_p) == -1) {
    perror("sem_post: Incorrect post of posix semaphore");
    exit(-1);
  };

  printf("Condition is set\n");

  return 0;
}

