/* Программа 01-sem-v-b.c для иллюстрации работы с  семафорами Эта программа
 * получает доступ к одному системному семафору и увеличивает его на 1
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  int semid; // IPC дескриптор для массива IPC семафоров

  /* Имя файла, использующееся для генерации ключа.
   * Файл с таким именем должен существовать в текущей директории
   */
  char pathname[] = ".";
  key_t key; // IPC ключ
  struct sembuf mybuf; // Структура для задания операции над семафором

  /* Генерируем IPC ключ из имени файла 08-1a.c в текущей
   *директории и номера экземпляра массива семафоров 0
   */
  if((key = ftok(pathname,0)) < 0) {
    printf("Can\'t generate key\n");
    exit(-1);
  }

  /* Пытаемся получить доступ по ключу к массиву семафоров, если он существует,
   * или создать его из одного семафора, если его еще не существует,
   * с правами доступа read & write для всех пользователей
   */
  if((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0){
    printf("Can\'t get semid\n");
    exit(-1);
  }

  /* Выполним операцию A(semid1,1) для нашего массива семафоров.
   * Для этого сначала заполним нашу структуру. Флаг, как обычно,
   * полагаем равным 0. Наш массив семафоров состоит из одного
   * семафора с номером 0. Код операции 1.
   */
  mybuf.sem_op = 1;
  mybuf.sem_flg = 0;
  mybuf.sem_num = 0;
  if(semop(semid, &mybuf, 1) < 0) {
    printf("Can\'t wait for condition\n");
    exit(-1);
  }

  int sem_value = semctl(semid, 0, GETVAL, 0);
  printf("The semaphore value = %d\n", sem_value);

  return 0;
}
