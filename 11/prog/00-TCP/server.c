#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Порт сервера, к этому порту должен подключаться клиент */
#define PORT 3333
/* Размер буфера */
#define BUF_SIZE 64
/* Приглашение сервера, его увидит клиент после подключения к серверу */
#define MSG_TO_SEND "My Simple Server v0.0.1\n"

int main () {
  /* Сокеты для клиента и сервера */
  int server_socket, client_socket;
  /* Длина ответа и счетчик клиентов */
  int answer_len, count=0;
  /* Буфер для чтения данных */
  char buffer[BUF_SIZE];
  /* Структуры sockaddr_in для клиента и сервера (sin) */
  struct sockaddr_in sin, client;

  /* Создаем сокет сервера */
  // server_socket = socket (AF_INET, SOCK_STREAM, 0);
  if ((server_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Failed to create socket\n");
    exit(1);
  }
  /* Заполняем память, см. man memset
  данная операция используется для очистки структуры */
  memset ((char *)&sin, '\0', sizeof(sin));
  /* Заполняем структуру sin */
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = PORT;

  /* Связываем сокет сервера с портом 3333 */
  // (void)bind (server_socket, (struct sockaddr *)&sin, sizeof(sin));
  if (bind (server_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    printf("Failed to bind the server socket\n");
    exit(1);
  }
  /* Сообщаем о своем запуске */
  printf("Server started.\n");
  /* Слушаем сокет сервера */
  listen (server_socket, 3);
  /* Запускаем бесконечный цикл для принятия запросов от клиентов,
     завершить работу программы можно, нажав <Ctrl>+<C> */
  while (1) {
    /* Вычисляем длину ответа */
    answer_len = sizeof(client);
    /* Принимаем запрос клиента */
    client_socket = accept (server_socket, (struct sockaddr *) &client,
                            (socklen_t *)&answer_len);
    if (client_socket < 0) {
      printf("Failed to accept client connection\n");
      exit(1);
    }

    /* Отправляем клиенту приветствие */
    write (client_socket, MSG_TO_SEND, sizeof(MSG_TO_SEND));
    /* Увеличиваем счетчик клиента */
    count++;
    /* Читаем ответ клиента в буфер,
    переменная anser_len будет содержать к-во прочитанных байтов */
    answer_len = read (client_socket, buffer, BUF_SIZE);
    /* Выводим на стандартный вывод ответ клиента */
    write (1, buffer, answer_len);
    /* Выводим порядковый номер клиента */
    // printf("Client no %d\n",count);
    printf("Client no %d connected: %s\n", count, inet_ntoa(client.sin_addr));
    /* Разрываем соединение */
    shutdown (client_socket, 0);
    /* Закрываем сокет */
    close (client_socket);
  };
  return 0;
}
