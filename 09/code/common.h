#ifndef COMMON_H
#define COMMON_H

// Заголовочный файл, содержащий общие данные для писателей и читателей
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 10  // Размер буфера ячеек
// Структура для хранения в разделяемой памяти буфера и необходимых данных
// Задает буфер, доступный процессам
typedef struct {
  int store[BUF_SIZE];   // буфер для заполнения ячеек
  int head;              // куда писать
  int tail;              // откуда читать
  pid_t writer_pid;      // идентификатор процесса читателя
  pid_t reader_pids[2];  // идентификаторы процессов читателей
  int reader_count;      // количесвто читателей.
} shared_memory;

// имя области разделяемой памяти
extern const char *shar_object;
extern int buf_id;             // дескриптор объекта памяти
extern shared_memory *buffer;  // указатель на разделямую память, хранящую буфер

// имя семафора для занятых ячеек
extern const char *full_sem_name;
extern sem_t *full;  // указатель на семафор занятых ячеек

// имя семафора для свободных ячеек
extern const char *empty_sem_name;
extern sem_t *empty;  // указатель на семафор свободных ячеек

// имя семафора (мьютекса) для критической секции для записи,
// обеспечивающей доступ к буферу
extern const char *writer_mutex_sem_name;
extern sem_t *writer_mutex;  // указатель на семафор читателей

// имя семафора (мьютекса) для критической секции для чтения,
// обеспечивающей доступ к буферу
extern const char *reader_mutex_sem_name;
extern sem_t *reader_mutex;  // указатель на семафор читателей

// Имя семафора для управления доступом.
// Позволяет читателю дождаться разрешения от читателя.
// Даже если читатель стартовал первым
extern const char *admin_sem_name;
extern sem_t *admin;  // указатель на семафор читателей

// Функция осуществляющая при запуске общие манипуляции с памятью и семафорами
// для децентрализованно подключаемых процессов читателей и писателей.
void init(void);

// Функция закрывающая семафоры общие для писателей и читателей
void close_common_semaphores(void);

// Функция, удаляющая все семафоры и разделяемую память
void unlink_all(void);

#endif