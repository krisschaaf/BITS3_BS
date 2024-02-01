#include "main.h"
#include "producer.h"
#include "consumer.h"
#include "observer.h"
#include "qeue.h"

#define consumerThreads 50
#define producerThreads 50

pthread_cond_t mainProdCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t mainConsCond = PTHREAD_COND_INITIALIZER;

int packInQeue = -1;              
int qeueFilled = 0;
int consumedTotal = 0;
int producedTotal = 0;

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int main(void)
{
    printf("Start \n\n");

    qeueSlot *root = initQeue();

    pthread_t produce_t[producerThreads];
    pthread_t consume_t[consumerThreads];
    pthread_t observer_t;

    pthread_mutex_t mainMutex = PTHREAD_MUTEX_INITIALIZER;

    //Erstelle alle Producer Threads
    for(int i=0; i < producerThreads; i++){
        pthread_create(&produce_t[i], NULL, &prodMain, root);
    }

    //Erstelle alle Consumer Threads
    for(int i=0; i < consumerThreads; i++){
        pthread_create(&consume_t[i], NULL, &consMain, root);
    }
    

    //Erstelle Observer Thread
    pthread_create(&observer_t, NULL, &observe, root);

    // Warte auf Erreichen der 1000 produzierten Pakete
    pthread_mutex_lock(&mainMutex);
    while(producedTotal < 1000) {
        pthread_cond_wait(&mainProdCond, &mainMutex);
    }
    pthread_mutex_unlock(&mainMutex);

    // Terminiere alle Producer Threads
    for(int i=0; i < producerThreads; i++){
        pthread_cancel((produce_t[i]));
    }

    // Warte auf Erreichen der 1000 konsumierten Pakete
    pthread_mutex_lock(&mainMutex);
    while(consumedTotal < 1000) {
        pthread_cond_wait(&mainConsCond, &mainMutex);
    }
    pthread_mutex_unlock(&mainMutex);

    // Terminiere alle Consumer Threads
    for(int i=0; i < consumerThreads; i++){
        pthread_cancel((consume_t[i]));
    }

    //Warte auf Terminierung des Observer Threads
    pthread_join(observer_t, NULL);

    //Programm Ende
}