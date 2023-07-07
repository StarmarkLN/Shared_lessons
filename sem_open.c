#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>

#define SEMAPHORE_NAME "/my_named_semaphore"

int main(int argc, char ** argv) {
    sem_t *sem;

    if ( argc == 2 ) {
        printf("Dropping semaphore...\n");
        if ( (sem = sem_open(SEMAPHORE_NAME, 0)) == SEM_FAILED ) {
            perror("sem_open");
            return 1;
        }
        sem_post(sem);
        perror("sem_post");
        printf("Semaphore dropped.\n");
        return 0;
    }

    if ( (sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) {
        perror("sem_open");
        return 1;
    }

    printf("Semaphore is taken.\nWaiting for it to be dropped.\n");
    sem_wait(sem);
    perror("sem_wait");
    sem_close(sem);
    perror("sem_close");

    return 0;
}


/*
Семафор

Семафор — самый часто употребляемый метод для синхронизации потоков и 
для контролирования одновременного доступа множеством потоков/процессов к 
общей памяти (к примеру, глобальной переменной). 
Взаимодействие между процессами в случае с семафорами заключается в том, 
что процессы работают с одним и тем же набором данных и корректируют свое поведение в зависимости от этих данных.

Есть два типа семафоров:

    семафор со счетчиком (counting semaphore), определяющий лимит ресурсов для процессов, получающих доступ к ним
    бинарный семафор (binary semaphore), имеющий два состояния «0» или «1» (чаще: «занят» или «не занят»)

Рассмотрим оба типа семафоров.

Семафор со счетчиком

Смысл семафора со счетчиком в том, 
чтобы дать доступ к какому-то ресурсу только определенному количеству процессов. 
Остальные будут ждать в очереди, когда ресурс освободится.

Итак, для реализации семафоров будем использовать POSIX функцию sem_open():

#include <semaphore.h>

sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);


В функцию для создания семафора мы передаем имя семафора, 
построенное по определенным правилам и управляющие флаги. 
Таким образом у нас получится именованный семафор.
Имя семафора строится следующим образом: 
	в начале идет символ "/" (косая черта), а следом латинские символы. 
	Символ «косая черта» при этом больше не должен применяться. 
	Длина имени семафора может быть вплоть до 251 знака.

Если нам необходимо создать семафор, то передается управляющий флаг O_CREATE. 
Чтобы начать использовать уже существующий семафор, то oflag равняется нулю. 
Если вместе с флагом O_CREATE передать флаг O_EXCL, 
то функция sem_open() вернет ошибку, в случае если семафор с указанным именем уже существует.

Параметр mode задает права доступа таким же образом, 
как это объяснено в предыдущих главах. 
А переменной value инициализируется начальное значение семафора. 
Оба параметра mode и value игнорируются в случае, 
когда семафор с указанным именем уже существует, а sem_open() вызван вместе с флагом O_CREATE.

Для быстрого открытия существующего семафора используем конструкцию:

#include <semaphore.h>

sem_t *sem_open(const char *name, int oflag);

, где указываются только имя семафора и управляющий флаг.

Пример семафора со счетчиком

Рассмотрим пример использования семафора для синхронизации процессов. 
В нашем примере один процесс увеличивает значение семафора и ждет, 
когда второй сбросит его, чтобы продолжить дальнейшее выполнение.

sem_open.c

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>

#define SEMAPHORE_NAME "/my_named_semaphore"

int main(int argc, char ** argv) {
    sem_t *sem;

    if ( argc == 2 ) {
        printf("Dropping semaphore...\n");
        if ( (sem = sem_open(SEMAPHORE_NAME, 0)) == SEM_FAILED ) {
            perror("sem_open");
            return 1;
        }
        sem_post(sem);
        perror("sem_post");
        printf("Semaphore dropped.\n");
        return 0;
    }

    if ( (sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) {
        perror("sem_open");
        return 1;
    }

    printf("Semaphore is taken.\nWaiting for it to be dropped.\n");
    if (sem_wait(sem) < 0 )
        perror("sem_wait");
    if ( sem_close(sem) < 0 )
        perror("sem_close");

    return 0;
}

[скачать]

Компилировать с флагом -lpthread  !!!
$ g++ -o sem_open sem_open.c -lpthread


В одной консоли запускаем:

$ ./sem_open 
Semaphore is taken.
Waiting for it to be dropped.       <-- здесь процесс в ожидании другого процесса
sem_wait: Success
sem_close: Success


В соседней консоли запускаем:

$ ./sem_open 1
Dropping semaphore...
sem_post: Success
Semaphore dropped.
*/

