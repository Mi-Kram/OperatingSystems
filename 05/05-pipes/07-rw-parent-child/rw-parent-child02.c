// rw-parent-child02.c
// Программа, состоящая из двух процессов.
// Родительский процесс создает именованный канал
// и пишет в него информацию для внешнего процесса
// Потомок принимает сообщение от внешнего процесса.
// Делается попытка аналогичным образом построить две
// программы, обменивающихся между собой данными
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int     my_fifo_fd;
    int     ext_fifo_fd;
    size_t  size;
    char my_msg[] = "Hello from 02!";
    int msg_size = sizeof(my_msg);
    int buf_size = 20;
    char    str_buf[buf_size];
    char    my_fifo_name[] = "rw02.fifo";
    char    ext_fifo_name[] = "rw01.fifo";

    int result = fork();  // Формирование дочернего процесса
    if(result < 0) {
        printf("Can\'t fork child\n");
        exit(-1);
    } else if (result > 0) { // Родитель. Пишет в свой канал
        (void)umask(0);
        // Вариант с проверкой требует удаления именованного канала
        // if(mknod(my_fifo_name, S_IFIFO | 0666, 0) < 0){
        //   printf("Can\'t create FIFO\n");
        //   exit(-1);
        // }

        // Вариант без проверки позволяет использовать уже созданный FIFO
        mknod(my_fifo_name, S_IFIFO | 0666, 0);
        if((my_fifo_fd = open(my_fifo_name, O_WRONLY)) < 0) {
            printf("rw02-parent: Can\'t open my FIFO for writting\n");
            exit(-1);
        }
        size = write(my_fifo_fd, my_msg, msg_size);
        if(size != msg_size) {
            printf("rw02-parent: Can\'t write all my string to my FIFO\n");
            exit(-1);
        }
        if(close(my_fifo_fd) < 0) {
            printf("rw02-parent: Can\'t close my FIFO\n");
            exit(-1);
        }
        printf("rw02-parent exit\n");
    } else { // Потомок. Читает из чужого канала
        if((ext_fifo_fd = open(ext_fifo_name, O_RDONLY)) < 0){
            printf("rw02-child: Can\'t open external FIFO for reading\n");
            exit(-1);
        }
        size = read(ext_fifo_fd, str_buf, buf_size);
        if(size < 0) {
            printf("rw02-child: Can\'t read external string from FIFO\n");
            exit(-1);
        }
        printf("rw02-child: exit, str_buf: %s\n", str_buf);
        if(close(ext_fifo_fd) < 0){
            printf("rw02-child: Can\'t close external FIFO\n");
            exit(-1);
        }
        printf("rw02-child exit\n");
    }
    return 0;
}
