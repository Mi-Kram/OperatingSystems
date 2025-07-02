// 08-rw-parent-child.c
// Программа, состоящая из двух процессов.
// Родительский процесс создает именованный канал
// и пишет в него информацию для внешнего процесса
// Потомок принимает сообщение от внешнего процесса.
// Реализовано взаимодействие двух одинаковых программ.
// Каждая из них получает три аргумента:
// первый - свой FIFO канал для передачи информации;
// второй - чужой FIFO канал для получения информации;
// третий - сообщение, передаваемое другой запущенной программе.
// Сообщение не должно содержать пробелов или быть в кавычках
// Длина сообщения не должна превышать 50 символов.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int     my_fifo_fd;
    int     ext_fifo_fd;
    size_t  size;
    // char my_msg[] = "Hello from 01!";
    // int msg_size = sizeof(my_msg);
    int buf_size = 60;
    char    str_buf[buf_size]; // буфер сообщения (с запасом ;)
    // char    my_fifo_name[buf_size]; // имя канала
    // char    ext_fifo_name[] = "rw02.fifo";

    // Проверка аргументов командной строки
    if(argc != 4) { // что-то пошло не так
        printf("Incorrect number of arguments = %d.\n", argc);
        exit(3);
    }
    int result = fork();  // Формирование дочернего процесса
    if(result < 0) {
        printf("Can\'t fork child\n");
        exit(-1);
    } else if (result > 0) { // Родитель. Пишет в свой канал
        (void)umask(0);
        // Вариант с проверкой требует удаления именованного канала
        // if(mknod(argv[1], S_IFIFO | 0666, 0) < 0){
        //   printf("Can\'t create FIFO\n");
        //   exit(-1);
        // }

        // Вариант без проверки позволяет использовать уже созданный FIFO
        mknod(argv[1], S_IFIFO | 0666, 0);
        if((my_fifo_fd = open(argv[1], O_WRONLY)) < 0) {
            printf("Can\'t open my FIFO = %s for writting\n", argv[1]);
            exit(-1);
        }
        int msg_size = strlen(argv[3]);
        // Проверка длины сообщения (чтобы разместилось в буфере принимающего)
        if(msg_size > 50) {
            printf("Message length = %d so long.\n", msg_size);
            exit(4);
        }
        size = write(my_fifo_fd, argv[3], msg_size);
        if(size != msg_size) {
            printf("Can\'t write my string to FIFO %s\n", argv[1]);
            exit(-1);
        }
        if(close(my_fifo_fd) < 0) {
            printf("Can\'t close my FIFO %s\n", argv[1]);
            exit(-1);
        }
        printf("Parent %s --> %s exit\n", argv[1], argv[2]);
    } else { // Потомок. Читает из чужого канала
        if((ext_fifo_fd = open(argv[2], O_RDONLY)) < 0) {
            printf("child: Can\'t open FIFO %s for reading\n", argv[2]);
            exit(-1);
        }
        size = read(ext_fifo_fd, str_buf, buf_size);
        if(size < 0) {
            printf("child: Can\'t read string from FIFO %s\n", argv[2]);
            exit(-1);
        }
        printf("child: Exit, str_buf = \"%s\"\n", str_buf);
        if(close(ext_fifo_fd) < 0){
            printf("child: Can\'t close external FIFO %s\n", argv[2]);
            exit(-1);
        }
        printf("Child %s <-- %s exit\n", argv[1], argv[2]);
    }
    return 0;
}
