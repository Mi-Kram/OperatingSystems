/* Программа sem-posix-post.c для иллюстрации работы с  семафорами
 * Эта программа получает доступ к одному системному семафору
 * и увеличивает его на 1
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

int main() {
  char sem_name[] = "/posix-semaphore"; // имя семафора
  sem_t *p_sem;   // адрес семафора
  char count_sem_name[] = "/count-posix-semaphore"; // имя семафора
  sem_t *p_count_sem;   // адрес семафора

  // Создание семафора для подсчета числа запусков процесса
  if((p_count_sem = sem_open(count_sem_name, O_CREAT, 0666, 5)) == 0) {
    perror("sem_open: Can not create posix semaphore");
    exit(-1);
  };
  // Проверка значения семафора-счетчика при каждом запуске
  int count;
  sem_getvalue(p_count_sem, &count);
  printf("count = %d\n", count);
  if(count > 1)  {
    sem_wait(p_count_sem);
    return count;
  }
  // Создание семафора
  if((p_sem = sem_open(sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: Can not create posix semaphore");
    exit(-1);
  };

  // Увеличение значения семафора на 1
  if(sem_post(p_sem) == -1) {
    perror("sem_post: Incorrect post of posix semaphore");
    exit(-1);
  };

  if(sem_close(p_sem) == -1) {
    perror("sem_close: Incorrect close of posix semaphore");
    exit(-1);
  };

  printf("Condition is set\n");

  // Удаление семафора-счетчика из памяти
  if(sem_close(p_count_sem) == -1) {
    perror("sem_close: Incorrect close of posix semaphore");
    exit(-1);
  };
  if(sem_unlink(count_sem_name) == -1) {
    perror("sem_unlink: Incorrect unlink of posix semaphore");
    exit(-1);
  };


  return 0;
}

