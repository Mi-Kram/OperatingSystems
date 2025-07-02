// Общий модуль, осуществляющий одинаковые административные функции
// как для писателя, так и для читателя.
#include "common.h"
#include <semaphore.h>

// имя области разделяемой памяти
const char* shar_object = "/posix-shar-object";
int buf_id;        // дескриптор объекта памяти
shared_memory *buffer;    // указатель на разделямую память, хранящую буфер

// имя семафора для занятых ячеек
const char *full_sem_name = "/full-semaphore";
sem_t *full;   // указатель на семафор занятых ячеек

// имя семафора для свободных ячеек
const char *empty_sem_name = "/empty-semaphore";
sem_t *empty;   // указатель на семафор свободных ячеек

// имя семафора (мьютекса) для чтения данных из буфера
const char *writer_mutex_sem_name = "/writer-mutex-semaphore";
sem_t *writer_mutex;   // указатель на семафор читателей

// имя семафора (мьютекса) для чтения данных из буфера
const char *reader_mutex_sem_name = "/reader-mutex-semaphore";
sem_t *reader_mutex;   // указатель на семафор читателей

// Имя семафора для подсчета контроля за запуском процессов
const char *admin_sem_name = "/admin-semaphore";
sem_t *admin;   // указатель на семафор читателей

// Функция осуществляющая при запуске общие манипуляции с памятью и семафорами
// для децентрализованно подключаемых процессов читателей и писателей.
void init(void) {
  // Создание или открытие семафора для администрирования (доступ открыт)
  if((admin = sem_open(admin_sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: Can not create admin semaphore");
    exit(-1);
  };
  // Создание или открытие мьютекса для доступа к буферу (доступ открыт)
  if((writer_mutex = sem_open(writer_mutex_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: Can not create writer mutex semaphore");
    exit(-1);
  };  
  // Создание или открытие мьютекса для доступа к буферу (доступ открыт)
  if((reader_mutex = sem_open(reader_mutex_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: Can not create reader mutex semaphore");
    exit(-1);
  };
  // Количество свободных ячеек равно BUF_SIZE
  if((empty = sem_open(empty_sem_name, O_CREAT, 0666, BUF_SIZE)) == 0) {
    perror("sem_open: Can not create free semaphore");
    exit(-1);
  };
  // Количество занятых ячеек равно 0
  if((full = sem_open(full_sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: Can not create busy semaphore");
    exit(-1);
  };
}

// Функция закрывающая семафоры общие для писателей и читателей
void close_common_semaphores(void) {
  sem_close(empty);
  sem_close(full);
  sem_close(admin);
  sem_close(writer_mutex);  
  sem_close(reader_mutex);
}

// Функция, удаляющая все семафоры и разделяемую память
void unlink_all(void) {
  sem_unlink(writer_mutex_sem_name);
  sem_unlink(reader_mutex_sem_name);
  sem_unlink(empty_sem_name);
  sem_unlink(full_sem_name);
  sem_unlink(admin_sem_name);
  shm_unlink(shar_object);
}

