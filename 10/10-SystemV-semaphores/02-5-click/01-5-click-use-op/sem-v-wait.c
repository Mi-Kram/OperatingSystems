/* Программа 01-sem-v-a.c для иллюстрации работы с семафорами */
/* Эта программа получает доступ к одному системному семафору,
ждет, пока его значение не станет больше или равным 1
после запусков программы 08-1b.c,а затем уменьшает его на 1*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  int semid; // IPC дескриптор для массива IPC семафоров

  /* Имя файла,  использующееся для генерации ключа.
   * Файл с таким именем должен существовать в текущей директории
   */
  char pathname[] = ".";
  key_t key; // IPC ключ
  struct sembuf mybuf; // Структура для задания операции над семафором

  /* Генерируем IPC-ключ из имени файла 08-1a.c в текущей
   * директории и номера экземпляра массива семафоров 0
   */
  if((key = ftok(pathname,0)) < 0) {
    printf("Can\'t generate key\n");
    exit(-1);
  }

  /* Пытаемся получить доступ по ключу к массиву семафоров, если он существует,
   * или создать его из одного семафора, если его еще не существует,
   * с правами доступа read & write для всех пользователей */
  if((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
    printf("Can\'t get semid\n");
    exit(-1);
  }

  /* Выполним операцию D(semid1,1) для нашего массива семафоров.
   * Для этого сначала заполним нашу структуру. Флаг, как обычно,
   * полагаем равным 0. Наш массив семафоров
   * состоит из одного семафора с номером 0. Код операции -5.
   */
  mybuf.sem_op = -5;
  mybuf.sem_flg = 0;
  mybuf.sem_num = 0;
  if(semop(semid, &mybuf, 1) < 0){
    printf("Can\'t wait for condition\n");
    exit(-1);
  }
  printf("Condition is present\n");

  int err = semctl(semid, 0, IPC_RMID, 0);
  if(err < 0) {
    printf("Incorrect semaphour destroy\n");
  }
  return 0;
}
