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
  const char* count_shar_object = "/posix-count-shar-object";
  int count_shmid;            // дескриптор разделяемой памяти для счетчика
  int *p_count;

  // Открыть объект памяти семафора ожидания
  if ( (shmid = shm_open(shar_object, O_RDWR, 0666)) == -1 ) {
    printf("Opening error\n");
    perror("shm_open");
    return 1;
  } else {
    printf("Object is open: name = %s, id = 0x%x\n", shar_object, shmid);
  }

  // Попытка открыть уже существующий объект данных для счетчика
  if ( (count_shmid = shm_open(count_shar_object, O_RDWR, 0666)) == -1 ) {
    // Объект отсутствует. Его нужно создать и инициализировать
    if ( (count_shmid = shm_open(count_shar_object,
                      O_CREAT|O_RDWR|O_EXCL, 0666)) == -1 ) {
      perror("shm_open: object is already open");
      exit(-1);
    } else {
      // При создании объекта осуществляется его начальная инициализация.
      // Задание размера объекта памяти для счетчика
      if (ftruncate(count_shmid, sizeof (int)) == -1) {
        perror("ftruncate: count memory sizing error");
        exit(-1);
      }
      // Получение доступа к разделяемой памяти счетчика
      p_count = mmap(0, sizeof (int),
                  PROT_WRITE|PROT_READ, MAP_SHARED, count_shmid, 0);
      *p_count = 5;
      printf("Start Count = %d\n", *p_count);
      return *p_count;
    }
  }
  // Объект существует. Нужно связаться
  // Получение доступа к разделяемой памяти счетчика
  p_count = mmap(0, sizeof (int),
                    PROT_WRITE|PROT_READ, MAP_SHARED, count_shmid, 0);

  --(*p_count);
  printf("Next Count = %d\n", *p_count);
  if (*p_count > 1) {
    return *p_count;
  }

  // Получение доступа к разделяемой памяти семафора для допуска
  sem_t *sem_p = mmap(0, sizeof (sem_t), PROT_WRITE|PROT_READ, MAP_SHARED, shmid, 0);

  // Увеличение значения семафора на 1
  if(sem_post(sem_p) == -1) {
    perror("sem_post: Incorrect post of posix semaphore");
    exit(-1);
  };

  printf("Condition is set\n");

  // удаление разделяемой памяти счетчика
  if(shm_unlink(count_shar_object) == -1) {
    perror("shm_unlink");
    exit(-1);
  }

  return 0;
}

