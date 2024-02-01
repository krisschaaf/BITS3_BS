#include "qeue.h"

pthread_mutex_t queueMutex[102];
pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;

// erzeugt Liste mit 25 leeren Paketen
//@return Pointer auf ersten qeueSlot (ID -1)
qeueSlot *initQeue()
{
    qeueSlot *firstSlot = malloc(sizeof(qeueSlot)); // reserviert Speicher der Größe des structs entsprechend
    firstSlot->paketID = -1;

    qeueSlot *lastSlot = malloc(sizeof(qeueSlot));
    lastSlot->paketID = 100;

    lastSlot->vorgaenger = firstSlot;
    firstSlot->nachfolger = lastSlot;

    // Initialisierung des queueMutex Arrays
    for (int i = 0; i < 102; i++)
    {
        pthread_mutex_t tempMutex = PTHREAD_MUTEX_INITIALIZER;

        queueMutex[i] = tempMutex;
    }

    packInQeue = 0;
    qeueFilled = 0;

    return firstSlot;
}

void produce(qeueSlot *firstSlot)
{
    qeueSlot *tempSlot = firstSlot;

    qeueSlot *tempVor;

    qeueSlot *newPackage = malloc(sizeof(qeueSlot));
    int tempID = rand() % 100;
    newPackage->paketID = tempID;

    PTHREAD_CANCEL_DISABLE;

    if (packInQeue < QMAX)
    {
        packInQeue++;
        producedTotal++;

        pthread_mutex_lock(&queueMutex[tempSlot->paketID + 1]); //firstSlot
        tempVor = tempSlot;
        tempSlot = tempSlot->nachfolger;
        pthread_mutex_lock(&queueMutex[tempSlot->paketID + 1]); //firstSlot -> Nachfolger 
        while (tempSlot->paketID < newPackage->paketID)
        {
            pthread_mutex_lock(&queueMutex[tempSlot->nachfolger->paketID + 1]); //firstSlot -> Nachfolger -> Nachfolger 
            pthread_mutex_unlock(&queueMutex[tempVor->paketID + 1]);    //firstSlot -> Nachfolger (unlock)
            tempVor = tempSlot;
            tempSlot = tempSlot->nachfolger;
        }

        if (newPackage->paketID != tempSlot->paketID)
        {
            // newPackage wird vor tempSlot in Warteschlange eingefügt
            pthread_mutex_lock(&queueMutex[tempID + 1]);
            (newPackage->vorgaenger) = tempVor;
            (newPackage->nachfolger) = tempSlot;

            tempVor->nachfolger = newPackage;
            (tempSlot->vorgaenger) = newPackage;

            qeueFilled = 1;

            pthread_mutex_unlock(&queueMutex[tempVor->paketID + 1]);    //tempVor
            pthread_mutex_unlock(&queueMutex[newPackage->paketID + 1]); //tempVor -> Nachfolger
            pthread_mutex_unlock(&queueMutex[tempSlot->paketID + 1]);   //tempVor -> Nachfolger -> Nachfolger 
            pthread_cond_signal(&queueCond);
            PTHREAD_CANCEL_ENABLE;
            msleep(200);
        }
        else
        {
            packInQeue--;
            producedTotal--;

            pthread_mutex_unlock(&queueMutex[tempVor->paketID + 1]);
            pthread_mutex_unlock(&queueMutex[newPackage->paketID + 1]);
            pthread_mutex_unlock(&queueMutex[tempSlot->paketID + 1]);
            PTHREAD_CANCEL_ENABLE;
            msleep(200);
        }
    }
    else
    {
        pthread_mutex_unlock(&queueMutex[tempID + 1]);
        PTHREAD_CANCEL_ENABLE;
        msleep(1000);
    }
    pthread_mutex_unlock(&queueMutex[tempID + 1]);
}

void consume(qeueSlot *firstSlot)
{

    PTHREAD_CANCEL_DISABLE;

    pthread_mutex_lock(&queueMutex[firstSlot->paketID + 1]);        //lock firstSlot 

    qeueSlot *consumerPack = firstSlot->nachfolger;

    while (consumerPack->paketID == 100)
    {
        PTHREAD_CANCEL_ENABLE;
        pthread_cond_wait(&queueCond, &queueMutex[firstSlot->paketID + 1]);
        PTHREAD_CANCEL_DISABLE;
        consumerPack = firstSlot->nachfolger;
    }

    pthread_mutex_lock(&queueMutex[consumerPack->paketID + 1]);     //firstSlot -> nachfolger 
    pthread_mutex_lock(&queueMutex[consumerPack->nachfolger->paketID + 1]); //firstSlot -> nachfolger -> nachfolger 

    packInQeue--;

    firstSlot->nachfolger = consumerPack->nachfolger;
    (consumerPack->nachfolger)->vorgaenger = firstSlot;

    consumedTotal++;

    pthread_mutex_unlock(&queueMutex[firstSlot->paketID + 1]);      //firstSlot   
    pthread_mutex_unlock(&queueMutex[consumerPack->paketID + 1]);   
    pthread_mutex_unlock(&queueMutex[firstSlot->nachfolger->paketID + 1]);
    free(consumerPack);

    PTHREAD_CANCEL_ENABLE;

    int lower = 500;
    int upper = 2000;
    int randNb = (rand() % (upper - lower + 1)) + lower;

    msleep(randNb);
}