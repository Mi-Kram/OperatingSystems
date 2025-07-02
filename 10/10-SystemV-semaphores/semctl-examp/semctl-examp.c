// Программа иллюстрирует возможности системного вызова semctl()
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>

#define MAXSETSIZE 25

int main () {
  extern int errno;
  struct semid_ds semid_ds;
  int length, rtrn, i, c;
  int semid, semnum, cmd, choice;
  union semun {
    int val;
    struct semid_ds *buf;
    ushort array [MAXSETSIZE];
  } arg;

  /* Инициализация указателя на структуру данных */
  arg.buf = &semid_ds;

  /* Ввести идентификатор множества семафоров */
  printf ("Введите ид-р множества семафоров: ");
  scanf ("%d", &semid);

  /* Выбрать требуемое управляющее действие */
  printf ("\nВведите номер требуемого действия:\n");
  printf ("  GETVAL   = 1\n");
  printf ("  SETVAL   = 2\n");
  printf ("  GETPID   = 3\n");
  printf ("  GETNCNT  = 4\n");
  printf ("  GETZCNT  = 5\n");
  printf ("  GETALL   = 6\n");
  printf ("  SETALL   = 7\n");
  printf ("  IPC_STAT = 8\n");
  printf ("  IPC_SET  = 9\n");
  printf ("  IPC_RMID = 10\n");
  printf ("  Выбор    = ");
  scanf ("%d", &cmd);

  /* Проверить значения */
  printf ("идентификатор = %d, команда = %d\n",
          semid, cmd);

  /* Сформировать аргументы и выполнить вызов */
  switch (cmd) {
    case 1:     /* Получить значение */
      printf ("\nВведите номер семафора: ");
      scanf ("%d", &semnum);
      /* Выполнить системный вызов */
      rtrn = semctl (semid, semnum, GETVAL, 0);
      printf ("\nЗначение семафора = %d\n", rtrn);
      break;

    case 2:     /* Установить значение */
      printf ("\nВведите номер семафора: ");
      scanf ("%d", &semnum);
      printf ("\nВведите значение: ");
      scanf ("%d", &arg.val);
      /* Выполнить системный вызов */
      rtrn = semctl (semid, semnum, SETVAL, arg.val);
      break;

    case 3:     /* Получить ид-р процесса */
      rtrn = semctl (semid, 0, GETPID, 0);
      printf ("\nПоследнюю операцию выполнил: %d\n",rtrn);
      break;

    case 4:    /* Получить число процессов, ожидающих
                  увеличения значения семафора */
      printf ("\nВведите номер семафора: ");
      scanf ("%d", &semnum);
      /* Выполнить системный вызов */
      rtrn = semctl (semid, semnum, GETNCNT, 0);
      printf ("\nЧисло процессов = %d\n", rtrn);
      break;

    case 5:    /* Получить число процессов, ожидающих
                  обнуления значения семафора */
      printf ("Введите номер семафора: ");
      scanf ("%d", &semnum);
      /* Выполнить системный вызов */
      rtrn = semctl (semid, semnum, GETZCNT, 0);
      printf ("\nЧисло процессов = %d\n", rtrn);
      break;

    case 6:    /* Опросить все семафоры */
      /* Определить число семафоров в множестве */
      rtrn = semctl (semid, 0, IPC_STAT, arg.buf);
      length = arg.buf->sem_nsems;
      if (rtrn == -1)
        goto ERROR;
      /* Получить и вывести значения всех
          семафоров в указанном множестве */
      rtrn = semctl (semid, 0, GETALL, arg.array);
      for (i = 0; i < length; i++)
        printf (" %d", arg.array [i]);
      break;

    case 7:    /* Установить все семафоры */
      /* Определить число семафоров в множестве */
      rtrn = semctl (semid, 0, IPC_STAT, arg.buf);
      length = arg.buf->sem_nsems;
      if (rtrn == -1)
        goto ERROR;
      printf ("\nЧисло семафоров = %d\n", length);
      /* Установить значения семафоров множества */
      printf ("\nВведите значения:\n");
      for (i = 0; i < length; i++)
        scanf ("%hd", &arg.array [i]);
      /* Выполнить системный вызов */
      rtrn = semctl (semid, 0, SETALL, arg.array);
      break;

    case 8:     /* Опросить состояние множества */
      rtrn = semctl (semid, 0, IPC_STAT, arg.buf);
      printf ("\nИдентификатор пользователя = %d\n", arg.buf->sem_perm.uid);
      printf ("Идентификатор группы = %d\n", arg.buf->sem_perm.gid);
      printf ("Права на операции = 0%o\n",
              arg.buf->sem_perm.mode);
      printf ("Число семафоров в множестве = %lu\n",
              arg.buf->sem_nsems);
      printf ("Время последней операции = %lu\n",
              arg.buf->sem_otime);
      printf ("Время последнего изменения = %lu\n",
              arg.buf->sem_ctime);
      break;

    case 9:    /* Выбрать и изменить поле
                  ассоциированной структуры данных */
      /* Опросить текущее состояние */
      rtrn = semctl (semid, 0, IPC_STAT, arg.buf);
      if (rtrn == -1)
        goto ERROR;

      printf ("\nВведите номер поля, ");
      printf ("которое нужно изменить: \n");
      printf ("  sem_perm.uid  = 1\n");
      printf ("  sem_perm.gid  = 2\n");
      printf ("  sem_perm.mode = 3\n");
      printf ("  Выбор         = ");
      scanf ("%d", &choice);

      switch (choice) {
        case 1:         /* Изменить ид-р владельца */
          printf ("\nВведите ид-р владельца: ");
          scanf ("%d", &arg.buf->sem_perm.uid);
          printf ("\nИд-р владельца = %d\n",
                  arg.buf->sem_perm.uid);
          break;

        case 2:         /* Изменить ид-р группы */
          printf ("\nВведите ид-р группы = ");
          scanf ("%d", &arg.buf->sem_perm.gid);
          printf ("\nИд-р группы = %d\n",
                  arg.buf->sem_perm.uid);
          break;

        case 3:      /* Изменить права на операции */
          printf ("\nВведите восьмеричный код прав: ");
          scanf ("%o", &arg.buf->sem_perm.mode);
          printf ("\nПрава = 0%o\n",
                  arg.buf->sem_perm.mode);
          break;
      }

      /* Внести изменения */
      rtrn = semctl (semid, 0, IPC_SET, arg.buf);
      break;

    case 10:    /* Удалить ид-р множества семафоров и
                    ассоциированную структуру данных */
      rtrn = semctl (semid, 0, IPC_RMID, 0);
  }
  if (rtrn == -1) {
    /* Сообщить о неудачном завершении */
    ERROR:
    printf ("\nsemctl завершился неудачей!\n");
    printf ("\nКод ошибки = %d\n", errno);
  }
  else {
    printf ("\nmsgctl завершился успешно,\n");
    printf ("идентификатор semid = %d\n", semid);
  }

  exit (0);
}
