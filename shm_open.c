#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define SHARED_MEMORY_OBJECT_NAME "my_shared_memory"
#define SHARED_MEMORY_OBJECT_SIZE 50
#define SHM_CREATE 1
#define SHM_PRINT  3
#define SHM_CLOSE  4

void usage(const char * s) {
    printf("Usage: %s <create|write|read|unlink> ['text']\n", s);
}

int main (int argc, char ** argv) {
    int shm, len, cmd, mode = 0;
    char *addr;

    if ( argc < 2 ) {
        usage(argv[0]);
        return 1;
    }

    if ( (!strcmp(argv[1], "create") || !strcmp(argv[1], "write")) && (argc == 3) ) {
        len = strlen(argv[2]);
        len = (len<=SHARED_MEMORY_OBJECT_SIZE)?len:SHARED_MEMORY_OBJECT_SIZE;
        mode = O_CREAT;
        cmd = SHM_CREATE;
    } else if ( ! strcmp(argv[1], "print" ) ) {
        cmd = SHM_PRINT;
    } else if ( ! strcmp(argv[1], "unlink" ) ) {
        cmd = SHM_CLOSE;
    } else {
        usage(argv[0]);
        return 1;
    }

    if ( (shm = shm_open(SHARED_MEMORY_OBJECT_NAME, mode|O_RDWR, S_IRWXO|S_IRWXG|S_IRWXU)) == -1 ) {
        perror("shm_open");
        return 1;
    }

    if ( cmd == SHM_CREATE ) {
        if ( ftruncate(shm, SHARED_MEMORY_OBJECT_SIZE+1) == -1 ) {
            perror("ftruncate");
            return 1;
        }
    }

    if ( (addr = (char*)mmap(0, SHARED_MEMORY_OBJECT_SIZE+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0)) == (char*)-1 ) {
        perror("mmap");
        return 1;
    }

    switch ( cmd ) {
    case SHM_CREATE:
        memcpy(addr, argv[2], len);
        addr[len] = '\0';
        printf("Shared memory filled in. You may run '%s print' to see shared memory value.\n", argv[0]);
        break;
    case SHM_PRINT:
        printf("Got from shared memory: %s\n", addr);
        break;
    }

    munmap(addr, SHARED_MEMORY_OBJECT_SIZE);
    close(shm);

    if ( cmd == SHM_CLOSE ) {
        shm_unlink(SHARED_MEMORY_OBJECT_NAME);
    }

    return 0;
}

/*
Следующий код демонстрирует создание, изменение и удаление разделяемой памяти. 
Так же показывается как после создания разделяемой памяти, программа выходит, 
но при следующем же запуске мы можем получить к ней доступ, 
пока не выполнен shm_unlink().

shm_open.c

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define SHARED_MEMORY_OBJECT_NAME "my_shared_memory"
#define SHARED_MEMORY_OBJECT_SIZE 50
#define SHM_CREATE 1
#define SHM_PRINT  3
#define SHM_CLOSE  4

void usage(const char * s) {
    printf("Usage: %s <create|write|read|unlink> ['text']\n", s);
}

int main (int argc, char ** argv) {
    int shm, len, cmd, mode = 0;
    char *addr;

    if ( argc < 2 ) {
        usage(argv[0]);
        return 1;
    }

    if ( (!strcmp(argv[1], "create") || !strcmp(argv[1], "write")) && (argc == 3) ) {
        len = strlen(argv[2]);
        len = (len<=SHARED_MEMORY_OBJECT_SIZE)?len:SHARED_MEMORY_OBJECT_SIZE;
        mode = O_CREAT;
        cmd = SHM_CREATE;
    } else if ( ! strcmp(argv[1], "print" ) ) {
        cmd = SHM_PRINT;
    } else if ( ! strcmp(argv[1], "unlink" ) ) {
        cmd = SHM_CLOSE;
    } else {
        usage(argv[0]);
        return 1;
    }

    if ( (shm = shm_open(SHARED_MEMORY_OBJECT_NAME, mode|O_RDWR, 0777)) == -1 ) {
        perror("shm_open");
        return 1;
    }

    if ( cmd == SHM_CREATE ) {
        if ( ftruncate(shm, SHARED_MEMORY_OBJECT_SIZE+1) == -1 ) {
            perror("ftruncate");
            return 1;
        }
    }

    addr = mmap(0, SHARED_MEMORY_OBJECT_SIZE+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (char*)-1 ) {
        perror("mmap");
        return 1;
    }

    switch ( cmd ) {
    case SHM_CREATE:
        memcpy(addr, argv[2], len);
        addr[len] = '\0';
        printf("Shared memory filled in. You may run '%s print' to see value.\n", argv[0]);
        break;
    case SHM_PRINT:
        printf("Got from shared memory: %s\n", addr);
        break;
    }

    munmap(addr, SHARED_MEMORY_OBJECT_SIZE);
    close(shm);

    if ( cmd == SHM_CLOSE ) {
        shm_unlink(SHARED_MEMORY_OBJECT_NAME);
    }

    return 0;
}

//=============================================================================================
Для выделения разделяемой памяти будем использовать POSIX функцию shm_open():

#include <sys/mman.h>

int shm_open(const char *name, int oflag, mode_t mode);


Функция возвращает файловый дескриптор, который связан с объектом памяти. 
Этот дескриптор в дальнейшем можно использовать другими функциями (к примеру, mmap() или mprotect()).

Целостность объекта памяти сохраняется, включая все данные связанные с ним, 
до тех пор пока объект не отсоединен/удален (shm_unlink()). 
Это означает, что любой процесс может получить доступ к нашему объекту памяти 
(если он знает его имя) до тех пор, пока явно в одном из процессов мы не вызовем shm_unlink().

Переменная oflag является побитовым «ИЛИ» следующих флагов:

    O_RDONLY — открыть только с правами на чтение
    O_RDWR — открыть с правами на чтение и запись
    O_CREAT — если объект уже существует, то от флага никакого эффекта. 
			  Иначе, объект создается и для него выставляются права доступа в соответствии с mode.
    O_EXCL — установка этого флага в сочетании с O_CREATE приведет к возврату функцией shm_open ошибки, 
			 если сегмент общей памяти уже существует.

Как задается значение параметра mode подробно описано в предыдущем параграфе «передача сообщений».

После создания общего объекта памяти, 
мы задаем размер разделяемой памяти вызовом ftruncate(). 
На входе у функции файловый дескриптор нашего объекта и необходимый нам размер.



После создания объекта памяти мы установили нужный нам размер shared memory вызовом ftruncate(). 
Затем мы получили доступ к разделяемой памяти при помощи mmap(). 
(Вообще говоря, даже с помощью самого вызова mmap() можно создать разделяемую память. 
Но отличие вызова shm_open() в том, 
что память будет оставаться выделенной до момента удаления или перезагрузки компьютера.)

Компилировать код на этот раз нужно с опцией -lrt (ставить в конце!!!):

$ gcc -o shm_open shm_open.c -lrt


Смотрим что получилось:

$ ./shm_open create 'Hello, my shared memory!'
Shared memory filled in. You may run './shm_open print' to see value.
$ ./shm_open print
Got from shared memory: Hello, my shared memory!
$ ./shm_open create 'Hello!'
Shared memory filled in. You may run './shm_open print' to see value.
$ ./shm_open print
Got from shared memory: Hello!
$ ./shm_open close
$ ./shm_open print
shm_open: No such file or directory


Аргумент «create» в нашей программе мы используем как для создания разделенной памяти, 
так и для изменения ее содержимого.

Зная имя объекта памяти, 
мы можем менять содержимое разделяемой памяти. 
Но стоит нам вызвать shm_unlink(), 
как память перестает быть нам доступна и shm_open() без параметра O_CREATE 
возвращает ошибку «No such file or directory».
*/
