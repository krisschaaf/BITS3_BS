#include "producer.h"

void* prodMain(void *firstSlot)  {
   while(producedTotal < 1000) {
        produce(firstSlot);
    }
    pthread_cond_signal(&mainProdCond);

    return NULL;
}
