#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define NAMEDPIPE_NAME "/tmp/my_named_pp"
#define BUFSIZE        50

int main (int argc, char ** argv) {
    int fd, len;
    char buf[BUFSIZE];

    if ( mkfifo(NAMEDPIPE_NAME, 0777) ) {
        perror("str_mkfifo");
        return 1;
    }
    printf("%s is created\n", NAMEDPIPE_NAME);

    if ( (fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0 ) {
        perror("open");
        return 1;
    }
    printf("%s is opened\n", NAMEDPIPE_NAME);

    do {
        memset(buf, '\0', BUFSIZE);
        
        if ( (len = read(fd, buf, BUFSIZE-1)) <= 0 ) {
            perror("read");
            close(fd);
            remove(NAMEDPIPE_NAME);
            return 0;
        }
		
        printf("Incomming message (%d): %s\n", len, buf);

    } while ( 1 );
}

/*
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define NAMEDPIPE_NAME "/tmp/my_named_pip"
#define BUFSIZE        50

int main (int argc, char ** argv) {
    int fd, len;
    char buf[BUFSIZE];

    if ( mkfifo(NAMEDPIPE_NAME, 0777) ) {
        perror("mkfifo");
        return 1;
    }
    printf("%s is created\n", NAMEDPIPE_NAME);

    if ( (fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0 ) {
        perror("open");
        return 1;
    }
    printf("%s is opened\n", NAMEDPIPE_NAME);

    do {
        memset(buf, '\0', BUFSIZE);
        if ( (len = read(fd, buf, BUFSIZE-1)) <= 0 ) {
            perror("read");
            close(fd);
            remove(NAMEDPIPE_NAME);
            return 0;
        }
        printf("Incomming message (%d): %s\n", len, buf);
    } while ( 1 );
}
*/

/*
Для создания именованных каналов будем использовать функцию, mkfifo():

#include <sys/stat.h>

int mkfifo(const char *pathname, mode_t mode);


Функция создает специальный FIFO файл с именем pathname, 
а параметр mode задает права доступа к файлу.

Примечание: mode используется в сочетании с текущим значением umask следующим образом: 
	(mode & ~umask). Результатом этой операции и будет новое значение umask для создаваемого нами файла. 
	По этой причине мы используем 0777 (S_IRWXO | S_IRWXG | S_IRWXU), 
	чтобы не затирать ни один бит текущей маски.

Как только файл создан, любой процесс может открыть этот файл для чтения или записи также, 
как открывает обычный файл. 
Однако, для корректного использования файла, 
необходимо открыть его одновременно двумя процессами/потоками, 
одним для получение данных (чтение файла), другим на передачу (запись в файл).

В случае успешного создания FIFO файла, mkfifo() возвращает 0 (нуль). 
В случае каких либо ошибок, функция возвращает -1 и выставляет код ошибки в переменную errno.

Типичные ошибки, которые могут возникнуть во время создания канала:

    EACCES — нет прав на запуск (execute) в одной из директорий в пути pathname
    EEXIST — файл pathname уже существует, даже если файл — символическая ссылка
    ENOENT — не существует какой-либо директории, упомянутой в pathname, либо является битой ссылкой
    ENOSPC — нет места для создания нового файла
    ENOTDIR — одна из директорий, упомянутых в pathname, на самом деле не является таковой
    EROFS — попытка создать FIFO файл на файловой системе «только-на-чтение»

Чтение и запись в созданный файл производится с помощью функций read() и write().

Пример

mkfifo.c

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define NAMEDPIPE_NAME "/tmp/my_named_pipe"
#define BUFSIZE        50

int main (int argc, char ** argv) {
    int fd, len;
    char buf[BUFSIZE];

    if ( mkfifo(NAMEDPIPE_NAME, 0777) ) {
        perror("mkfifo");
        return 1;
    }
    printf("%s is created\n", NAMEDPIPE_NAME);

    if ( (fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0 ) {
        perror("open");
        return 1;
    }
    printf("%s is opened\n", NAMEDPIPE_NAME);

    do {
        memset(buf, '\0', BUFSIZE);
        if ( (len = read(fd, buf, BUFSIZE-1)) <= 0 ) {
            perror("read");
            close(fd);
            remove(NAMEDPIPE_NAME);
            return 0;
        }
        printf("Incomming message (%d): %s\n", len, buf);
    } while ( 1 );
}

[скачать]

Мы открываем файл только для чтения (O_RDONLY). 
И могли бы использовать O_NONBLOCK модификатор, 
предназначенный специально для FIFO файлов, 
чтобы не ждать когда с другой стороны файл откроют для записи. 
Но в приведенном коде такой способ неудобен.

Компилируем программу, затем запускаем ее:

$ gcc -o mkfifo mkfifo.c
$ ./mkfifo


В соседнем терминальном окне выполняем:

$ echo 'Hello, my named pipe!' > /tmp/my_named_pipe


В результате мы увидим следующий вывод от программы:

$ ./mkfifo 
/tmp/my_named_pipe is created
/tmp/my_named_pipe is opened
Incomming message (22): Hello, my named pipe!
read: Success



*/
