#include "consumer.h"

void* consMain(void *firstSlot)  {
    while(consumedTotal <= producedTotal && consumedTotal < 1000) {
        consume(firstSlot);
    }
    pthread_cond_signal(&mainConsCond);

    return NULL;
}