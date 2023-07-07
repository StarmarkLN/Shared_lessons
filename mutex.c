/*
Рабочий параметр в Ubuntu -lpthread. 
Но если вы работаете с suse или другими системами, правильный параметр -lrt. 
Также в книге Linux Programmin Interface упоминается -lrt как правильный вариант.
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

static int counter; // shared resource
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void incr_counter(void *p) {
    do {
        usleep(10); // Let's have a time slice between mutex locks
        pthread_mutex_lock(&mutex);
        counter++;
        printf("%d\n", counter);
        sleep(1);
        pthread_mutex_unlock(&mutex);
    } while ( 1 );
}

void reset_counter(void *p) {
    char buf[10];
    int  num = 0;
    int  rc;
    pthread_mutex_lock(&mutex); // block mutex just to show message
    printf("Enter the number and press 'Enter' to initialize the counter with new value anytime.\n");
    sleep(3);
    pthread_mutex_unlock(&mutex); // unblock blocked mutex so another thread may work
    do {
        if ( gets(buf) != buf ) return; // NO fool-protection ! Risk of overflow !
        num = atoi(buf);
        if ( (rc = pthread_mutex_trylock(&mutex)) == EBUSY ) {
            printf("Mutex is already locked by another process.\n\
Let's lock mutex using pthread_mutex_lock().\n");
            pthread_mutex_lock(&mutex);
        } else if ( rc == 0 ) {
            printf("WOW! You are on time! Congratulation!\n");
        } else { 
            printf("Error: %d\n", rc);
            return;
        }
        counter = num;
        printf("New value for counter is %d\n", counter);
        pthread_mutex_unlock(&mutex);
    } while ( 1 );
}

int main(int argc, char ** argv) {
    pthread_t thread_1;
    pthread_t thread_2;

    counter = 0;

    pthread_create(&thread_1, NULL, (void *)&incr_counter, NULL);
    pthread_create(&thread_2, NULL, (void *)&reset_counter, NULL);

    pthread_join(thread_2, NULL);

    return 0;
}


/*
Вместо бинарного семафора, для которого так же используется функция sem_open, 
я рассмотрю гораздо чаще употребляемый семафор, называемый «мьютекс» (mutex).

Мьютекс по существу является тем же самым, чем является бинарный семафор 
(т.е. семафор с двумя состояниями: «занят» и «не занят»). 
Но термин «mutex» чаще используется чтобы описать схему, 
которая предохраняет два процесса от одновременного использования общих данных/переменных. 
В то время как термин «бинарный семафор» чаще употребляется для описания конструкции, 
которая ограничивает доступ к одному ресурсу. 
То есть бинарный семафор используют там, где один процесс «занимает» семафор, а другой его «освобождает». 
В то время как мьютекс освобождается тем же процессом/потоком, который занял его.

Без мьютекса не обойтись в написании, к примеру базы данных, к которой доступ могут иметь множество клиентов.

Для использования мьютекса необходимо вызвать функцию pthread_mutex_init():

#include <pthread.h>

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);


Функция инициализирует мьютекс (перемнную mutex) аттрибутом mutexattr. 
Если mutexattr равен NULL, то мьютекс инициализируется значением по умолчанию. 
В случае успешного выполнения функции (код возрата 0), 
мьютекс считается инициализированным и «свободным».

Типичные ошибки, которые могут возникнуть:

    EAGAIN — недостаточно необходимых ресурсов (кроме памяти) для инициализации мьютекса
    ENOMEM — недостаточно памяти
    EPERM — нет прав для выполнения операции
    EBUSY — попытка инициализировать мьютекс, который уже был инициализирован, но не унечтожен
    EINVAL — значение mutexattr не валидно

Чтобы занять или освободить мьютекс, используем функции:

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);


Функция pthread_mutex_lock(), если mutex еще не занят, то занимает его, 
становится его обладателем и сразу же выходит. 
Если мьютекс занят, то блокирует дальнейшее выполнение процесса и ждет освобождения мьютекса.

Функция pthread_mutex_trylock() идентична по поведению функции pthread_mutex_lock(), 
с одним исключением — она не блокирует процесс, если mutex занят, а возвращает EBUSY код.

Фунция pthread_mutex_unlock() освобождает занятый мьютекс.

Коды возврата для pthread_mutex_lock():

    EINVAL — mutex неправильно инициализирован
    EDEADLK — мьютекс уже занят текущим процессом

Коды возврата для pthread_mutex_trylock():

    EBUSY — мьютекс уже занят
    EINVAL — мьютекс неправильно инициализирован

Коды возврата для pthread_mutex_unlock():

    EINVAL — мьютекс неправильно инициализирован
    EPERM — вызывающий процесс не является обладателем мьютекса

Пример mutex

mutex.c

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

static int counter; // shared resource
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void incr_counter(void *p) {
    do {
        usleep(10); // Let's have a time slice between mutex locks
        pthread_mutex_lock(&mutex);
        counter++;
        printf("%d\n", counter);
        sleep(1);
        pthread_mutex_unlock(&mutex);
    } while ( 1 );
}

void reset_counter(void *p) {
    char buf[10];
    int  num = 0;
    int  rc;
    pthread_mutex_lock(&mutex); // block mutex just to show message
    printf("Enter the number and press 'Enter' to initialize the counter with new value anytime.\n");
    sleep(3);
    pthread_mutex_unlock(&mutex); // unblock blocked mutex so another thread may work
    do {
        if ( gets(buf) != buf ) return; // NO fool-protection ! Risk of overflow !
        num = atoi(buf);
        if ( (rc = pthread_mutex_trylock(&mutex)) == EBUSY ) {
            printf("Mutex is already locked by another process.\nLet's lock mutex using pthread_mutex_lock().\n");
            pthread_mutex_lock(&mutex);
        } else if ( rc == 0 ) {
            printf("WOW! You are on time! Congratulation!\n");
        } else { 
            printf("Error: %d\n", rc);
            return;
        }
        counter = num;
        printf("New value for counter is %d\n", counter);
        pthread_mutex_unlock(&mutex);
    } while ( 1 );
}

int main(int argc, char ** argv) {
    pthread_t thread_1;
    pthread_t thread_2;
    counter = 0;

    pthread_create(&thread_1, NULL, (void *)&incr_counter, NULL);
    pthread_create(&thread_2, NULL, (void *)&reset_counter, NULL);

    pthread_join(thread_2, NULL);
    return 0;
}

[скачать]

Данный пример демонстрирует совместный доступ двух потоков к общей переменной. 
Один поток (первый поток) в автоматическом режиме постоянно увеличивает 
переменную counter на единицу, 
при этом занимая эту переменную на целую секунду. 
Этот первый поток дает второму доступ к переменной count только на 10 миллисекунд, 
затем снова занимает ее на секунду. 
Во втором потоке предлагается ввести новое значение для переменной с терминала.

Если бы мы не использовали технологию «мьютекс», 
то какое значение было бы в глобальной переменной, 
при одновременном доступе двух потоков, нам не известно. 
Так же во время запуска становится очевидна разница между pthread_mutex_lock() и pthread_mutex_trylock().

Компилировать код нужно с дополнительным параметром -lpthread:


$ gcc -o mutex -lpthread mutex.c


Запускаем и меняем значение переменной просто вводя новое значение в терминальном окне:

$ ./mutex 
Enter the number and press 'Enter' to initialize the counter with new value anytime.
1
2
3
30 <Enter>     <--- новое значение переменной
Mutex is already locked by another process.
Let's lock mutex using pthread_mutex_lock().
New value for counter is 30
31
32
33
1 <Enter>     <--- новое значение переменной
Mutex is already locked by another process.
Let's lock mutex using pthread_mutex_lock().
New value for counter is 1
2
3
*/
