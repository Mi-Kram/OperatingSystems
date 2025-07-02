// reader.c - читатель забирающий из первой по очереди занятой ячейки
#include <unistd.h>

#include "common.h"

// Семафор для отсекания лишних читателей
// То есть, для формирования только одного процесса-читателя
const char *reader_sem_name = "/reader-semaphore";
sem_t *reader;  // указатель на семафор пропуска читателей

void sigfunc(int sig) {
  if (sig != SIGINT && sig != SIGTERM) {
    return;
  }
  if (sig == SIGINT) {
    kill(buffer->writer_pid, SIGTERM);
    printf("Reader(SIGINT) ---> Writer(SIGTERM)\n");
  } else if (sig == SIGTERM) {
    printf("Reader(SIGTERM) <--- Writer(SIGINT)\n");
  }

  // Удаляем текущий pid из списка
  sem_wait(reader_mutex);
  for (int i = 0; i < 2; ++i) {
    if (buffer->reader_pids[i] == getpid()) {
      buffer->reader_pids[i] = -1;
      buffer->reader_count--;
      break;
    }
  }
  sem_post(reader_mutex);

  sem_close(reader);
  sem_unlink(reader_sem_name);
  close_common_semaphores();
  
  printf("Reader: bye!!!\n");
  exit(10);
}

// Функция вычисления факториала
int factorial(int n) {
  int p = 1;
  for (int i = 1; i <= n; ++i) {
    p *= i;
  }
  return p;
}

int main() {
  signal(SIGINT, sigfunc);
  signal(SIGTERM, sigfunc);

  srand(time(0));  // Инициализация генератора случайных чисел
  init();          // Начальная инициализация общих семафоров
  // printf("init checkout\n");

  // Если читатель запустился раньше, он ждет разрешения от администратора,
  // Который находится в писателе
  if (sem_wait(admin) == -1) {  // ожидание когда запустится писатель
    perror("sem_wait: Incorrect wait of admin semaphore");
    exit(-1);
  };
  printf("Consumer %d started\n", getpid());
  // После этого он вновь поднимает семафор, позволяющий запустить других читателей
  if (sem_post(admin) == -1) {
    perror("sem_post: Consumer can not increment admin semaphore");
    exit(-1);
  }

  // Читатель имеет доступ только к открытому объекту памяти,
  // но может не только читать, но и писать (забирать)
  if ((buf_id = shm_open(shar_object, O_RDWR, 0666)) == -1) {
    perror("shm_open: Incorrect reader access");
    exit(-1);
  } else {
    printf("Memory object is opened: name = %s, id = 0x%x\n", shar_object, buf_id);
  }
  // Задание размера объекта памяти
  if (ftruncate(buf_id, sizeof(shared_memory)) == -1) {
    perror("ftruncate");
    exit(-1);
  } else {
    printf("Memory size set and = %lu\n", sizeof(shared_memory));
  }

  // получить доступ к памяти
  buffer = mmap(0, sizeof(shared_memory), PROT_WRITE | PROT_READ, MAP_SHARED, buf_id, 0);
  if (buffer == (shared_memory *)-1) {
    perror("reader: mmap");
    exit(-1);
  }

  // Разборки читателей. Семафор для конкуренции за работу
  if ((reader = sem_open(reader_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: Can not create reader semaphore");
    exit(-1);
  };
  // Первый просочившийся запрещает доступ остальным за счет установки флага
  if (sem_wait(reader) == -1) {  // ожидание когда запустится писатель
    perror("sem_wait: Incorrect wait of reader semaphore");
    exit(-1);
  };

  if (sem_wait(reader_mutex) == -1) {
    perror("sem_wait: reader mutex");
    exit(-1);
  }

  if (buffer->reader_count >= 2) {
    sem_post(reader_mutex);
    sem_post(reader);
    printf("Reader %d: too many readers, exiting...\n", getpid());
    exit(13);
  }

  // Добавляем себя в список читателей
  for (int i = 0; i < 2; ++i) {
    if (buffer->reader_pids[i] == -1 || buffer->reader_pids[i] == 0) {
      buffer->reader_pids[i] = getpid();
      break;
    }
  }
  buffer->reader_count++;

  if (sem_post(reader_mutex) == -1) {
    perror("sem_post: reader mutex");
    exit(-1);
  }

  // Пропуск  читателей других для определения возможности поработать
  if (sem_post(reader) == -1) {
    perror("sem_post: Consumer can not increment reader semaphore");
    exit(-1);
  }

  // Алгоритм читателя
  while (1) {
    sleep(rand() % 3 + 1);
    // Контроль наличия элементов в буфере
    if (sem_wait(full) == -1) {
      perror("sem_wait: Incorrect wait of full semaphore");
      exit(-1);
    };
    // критическая секция
    if (sem_wait(reader_mutex) == -1) {
      perror("sem_wait: Incorrect wait of busy semaphore");
      exit(-1);
    };
    // Текущая ячейка для записи. Она должна быть. Иначе сюда не попасть
    int i = buffer->tail;
    // Получение значения из читаемой ячейки
    int result = buffer->store[i];
    buffer->store[i] = -1;  // данные прочитаны
    // Переход к следующей ячейке.
    buffer->tail = (buffer->tail + 1) % BUF_SIZE;
    // вычисление факториала
    int f = factorial(result);
    // количество свободных ячеек увеличилось на единицу
    if (sem_post(empty) == -1) {
      perror("sem_post: Incorrect post of free semaphore");
      exit(-1);
    };
    // Вывод информации об операции чтения
    pid_t pid = getpid();
    printf("Consumer %d: Reads value = %d from cell [%d], factorial = %d\n", pid, result, i, f);
    // Выход из критической секции
    if (sem_post(reader_mutex) == -1) {
      perror("sem_post: Incorrect post of mutex semaphore");
      exit(-1);
    };
  }
}
