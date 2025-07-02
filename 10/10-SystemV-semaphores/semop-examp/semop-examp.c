/* Программа иллюстрирует возможности системного вызова semop()
 * (операции над множеством семафоров)
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define MAXOPSIZE 10

int main () {
    extern int errno;
    struct sembuf sops [MAXOPSIZE];
    int semid, flags, i, rtrn;
    unsigned nsops;

    /* Ввести идентификатор множества семафоров */
    printf ("\nВведите идентификатор множества семафоров,");
    printf ("\nнад которым будут выполняться операции: ");
    scanf ("%d", &semid);
    printf ("\nИд-р множества семафоров = %d", semid);

    /* Ввести число операций */
    printf ("\nВведите число операций ");
    printf ("над семафорами из этого множества: \n");
    scanf ("%d", &nsops);
    printf ("\nЧисло операций = %d", nsops);

    /* Инициализировать массив операций */
    for (i = 0; i < nsops; i++) {
        /* Выбрать семафор из множества */
        printf ("\nВведите номер семафора: ");
        scanf ("%hd", &sops [i].sem_num);
        printf ("\nНомер = %d", sops [i].sem_num);

        /* Ввести число, задающее операцию */
        printf ("\nЗадайте операцию над семафором: ");
        scanf ("%hd", &sops [i].sem_op);
        printf ("\nОперация = %d", sops [i].sem_op);

        /* Указать требуемые флаги */
        printf ("\nВведите код, ");
        printf ("соответствующий требуемым флагам:\n");
        printf ("  Нет флагов            = 0\n");
        printf ("  IPC_NOWAIT            = 1\n");
        printf ("  SEM_UNDO              = 2\n");
        printf ("  IPC_NOWAIT и SEM_UNDO = 3\n");
        printf ("  Выбор                 = ");
        scanf ("%d", &flags);

        switch (flags) {
            case 0:
            sops [i].sem_flg = 0;
            break;
            case 1:
            sops [i].sem_flg = IPC_NOWAIT;
            break;
            case 2:
            sops [i].sem_flg = SEM_UNDO;
            break;
            case 3:
            sops [i].sem_flg = IPC_NOWAIT | SEM_UNDO;
            break;
        }
        printf ("\nФлаги = 0%o", sops [i].sem_flg);
    }

    /* Распечатать все структуры массива */
    printf ("\nМассив операций:\n");
    for (i = 0; i < nsops; i++) {
        printf ("  Номер семафора = %d\n", sops [i].sem_num);
        printf ("  Операция = %d\n", sops [i].sem_op);
        printf ("  Флаги = 0%o\n", sops [i].sem_flg);
    }

    /* Выполнить системный вызов */
    rtrn = semop (semid, sops, nsops);
    if (rtrn == -1) {
        printf ("\nsemop завершился неудачей!\n");
        printf ("Код ошибки = %d\n", errno);
    }
    else {
        printf ("\nsemop завершился успешно.\n");
        printf ("Идентификатор semid = %d\n", semid);
        printf ("Возвращенное значение = %d\n", rtrn);
    }

    return 0;
}
