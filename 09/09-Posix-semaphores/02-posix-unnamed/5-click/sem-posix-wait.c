// Программа sem-posix-wait.c
// для иллюстрации работы с неименованным семафором.
// Эта программа получает доступ к одному posix семафору,
// ждет, пока его значение не станет больше или равным 1
// после запусков программы 01-sem-posix-post.c, а затем уменьшает его на 1
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  const char* shar_object = "/posix-shar-object";
  int shmid;            // дескриптор разделяемой памяти

  // Создание объекта памяти
  if ( (shmid = shm_open(shar_object, O_CREAT|O_RDWR, 0666)) == -1 ) {
    perror("shm_open: object is already open");
    exit(-1);
  } else {
    printf("Object is open: name = %s, id = 0x%x\n", shar_object, shmid);
  }

  // Задание размера объекта памяти
  if (ftruncate(shmid, sizeof (sem_t)) == -1) {
    perror("ftruncate: memory sizing error");
    exit(-1);
  } else {
    printf("Memory size set and = %lu\n", sizeof (sem_t));
  }

  // Получение доступа к разделяемой памяти
  sem_t *sem_p = mmap(0, sizeof (sem_t), PROT_WRITE|PROT_READ, MAP_SHARED, shmid, 0);

  // Инициализация семафора в разделяемой памяти
  if (sem_init(sem_p, 1, 0) == -1) {
    perror("sem_init");
    exit(-1);
  }

  // Обнуленный семафор ожидает, когда его поднимут, чтобы вычесть 1
  if(sem_wait(sem_p) == -1) {
    perror("sem_wait: Incorrect wait of posix semaphore");
    exit(-1);
  };

  // Семафор дождался второго процесса
  printf("Condition is present\n");

  // удаление разделяемой памяти вместе с семафором
  if(shm_unlink(shar_object) == -1) {
    perror("shm_unlink");
    exit(-1);
  }

  return 0;
}

