#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER_HOST "localhost" // Имя сервера (доменное имя, не IP-адрес),

//номер порта сервера, номер порта клиента — должны отличаться
#define SERVER_PORT 3333
#define CLIENT_PORT 3334

// Строка, которая будет передана серверу
#define MSG "Hello, Server, I'm client!'\n"

int main () {
  int sock;                 // Дескриптор сокета
  int answer_len;           // Длина ответа сервера
  int BUF_SIZE = 64;        // Размер буфера
  char buffer[BUF_SIZE];    // Буфер
  // Структура для преобразования доменного имени в IP-адрес
  struct hostent *h;
  // Структуры адреса для клиента и сервера
  struct sockaddr_in client, server;

  // Создается сокет
  sock = socket(AF_INET, SOCK_STREAM, 0);
  // Очистка структуры client
  memset ((char *)&client, '\0', sizeof(client));
  // Заполнение структуры
  client.sin_family = AF_INET;
  client.sin_addr.s_addr = INADDR_ANY;
  client.sin_port = CLIENT_PORT;

  // Связывание сокета клиента с портом клиента
  (void)bind (sock, (struct sockaddr *)&client, sizeof(client));
  memset ((char *)&client, '\0', sizeof(server));
  // Получение IP-адреса сервера
  h = gethostbyname (SERVER_HOST);
  // Заполнение структуры адреса для сервера
  server.sin_family = AF_INET;
  // Установка адреса сервера
  memcpy ((char *)&server.sin_addr,h->h_addr,h->h_length);
  // Установка порта сервера
  server.sin_port = SERVER_PORT;

  // Подключение к серверу
  (void)connect (sock, (struct sockaddr *) &server, sizeof(server));
  // Получение приветствия от сервера */
  answer_len = recv (sock, buffer, BUF_SIZE, 0);
  // Вывод приветствия на стандартный вывод
  write (1, buffer, answer_len);
  // Отправка данных серверу
  send (sock, MSG, sizeof(MSG), 0);

  // Закрытие сокета и завершение работы программы
  shutdown (sock, 0);
  close (sock);
  exit (0);
}
