#ifndef QEUE_H
#define QEUE_H

#include "main.h"

typedef struct tupel{
    int vorgaengerID;
    int mittlereID;
    int nachfolgerID;
} tupel;

extern pthread_cond_t queueCond;

qeueSlot *initQeue(void);
void produce(qeueSlot *firstSlot);
void consume(qeueSlot *firstSlot);
tupel lockElementsCons(qeueSlot *consumerPack);
tupel lockElementsProd(qeueSlot *tempSlot, int tempID);
void unlockElements(tupel tempTupel);

#endif