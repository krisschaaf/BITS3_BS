#include "qeue.h"

pthread_mutex_t queueMutex[102];
pthread_mutex_t lockMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unlockMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;

//erzeugt Liste mit 25 leeren Paketen
//@return Pointer auf ersten qeueSlot (ID -1)
qeueSlot *initQeue()
{
    qeueSlot* firstSlot = malloc(sizeof(qeueSlot)); //reserviert Speicher der Größe des structs entsprechend
    firstSlot->paketID = -1;

    qeueSlot* lastSlot = malloc(sizeof(qeueSlot));
    lastSlot->paketID = 100;

    lastSlot->vorgaenger = firstSlot;
    firstSlot->nachfolger = lastSlot;

    //Initialisierung des queueMutex Arrays
    for (int i = 0; i <= 99; i++)
    {
        pthread_mutex_t tempMutex = PTHREAD_MUTEX_INITIALIZER;

        queueMutex[i] = tempMutex;
    }

    packInQeue = 0;
    qeueFilled = 0;

    return firstSlot;
}

void produce(qeueSlot* firstSlot)
{
    qeueSlot* tempSlot = firstSlot;

    qeueSlot* newPackage = malloc(sizeof(qeueSlot));
    int tempID = rand() % 100;
    newPackage->paketID = tempID;

    PTHREAD_CANCEL_DISABLE;
    pthread_mutex_lock(&queueMutex[tempID]);

    if (packInQeue < QMAX)
    {
        //pthread_mutex_lock(&queueMutex[tempSlot->paketID]);
        while (tempSlot->paketID < newPackage->paketID)
        {
            //pthread_mutex_lock(&queueMutex[tempSlot->nachfolger->paketID]);
            tempSlot = tempSlot->nachfolger;
            //pthread_mutex_unlock(&queueMutex[tempSlot->vorgaenger->paketID]);
        }

        //pthread_mutex_lock(&queueMutex[tempSlot->vorgaenger->paketID]);

        if (newPackage->paketID != tempSlot->paketID)
        {   
            //pthread_mutex_lock(&lockMutex);
            tupel tempTupel = lockElementsProd(tempSlot, tempID);
            //pthread_mutex_unlock(&lockMutex);
            //newPackage wird vor tempSlot in Warteschlange eingefügt
            (newPackage->vorgaenger) = (tempSlot->vorgaenger);
            (newPackage->nachfolger) = tempSlot;

            //Sonderfall für erstes elem einfuegen
            qeueSlot* tempVorgaenger = tempSlot->vorgaenger;
            if(tempVorgaenger->paketID == -1){
                firstSlot->nachfolger = newPackage;
            } else {
                (tempVorgaenger->nachfolger) = newPackage;
            }
            (tempSlot->vorgaenger) = newPackage;

            packInQeue++;
            producedTotal++;
            qeueFilled = 1;
            pthread_cond_signal(&queueCond);

            pthread_mutex_lock(&unlockMutex);
            //tupel tempTupel = {newPackage->vorgaenger->paketID, tempID, newPackage->nachfolger->paketID};
            unlockElements(tempTupel);
            pthread_mutex_unlock(&unlockMutex);           
            PTHREAD_CANCEL_ENABLE;
            msleep(200);
        } else {
            //printf("Wieder gleiche Nummer!\n");
        }
    }
    else
    {
        pthread_mutex_unlock(&queueMutex[tempID]);
        PTHREAD_CANCEL_ENABLE;
        msleep(1000);
    }
    pthread_mutex_unlock(&queueMutex[tempID]);
}

void consume(qeueSlot *firstSlot)
{
    
    
    PTHREAD_CANCEL_DISABLE;

    //pthread_mutex_lock(&lockMutex);

    

    while (packInQeue == 0)
    {
        PTHREAD_CANCEL_ENABLE;
        pthread_cond_wait(&queueCond, &lockMutex);
        PTHREAD_CANCEL_DISABLE;
    }

    qeueSlot *consumerPack = firstSlot->nachfolger;

    tupel tempTupel = lockElementsCons(consumerPack);

    

    (consumerPack->vorgaenger)->nachfolger = consumerPack->nachfolger;
    (consumerPack->nachfolger)->vorgaenger = consumerPack->vorgaenger;

    free(consumerPack);

    //pthread_mutex_unlock(&lockMutex);

    packInQeue--;
    consumedTotal++;

    pthread_mutex_lock(&unlockMutex);
    unlockElements(tempTupel);
    pthread_mutex_unlock(&unlockMutex);
    PTHREAD_CANCEL_ENABLE;

    int lower = 500;
    int upper = 2000;
    int randNb = (rand() % (upper - lower + 1)) + lower;

    msleep(randNb);
    //printf("Bearbeitungszeit für ConsumerPack: %dms\n", randNb);
}

//sperrt Vorgaenger- und Nachfolgeelemente
tupel lockElementsCons(qeueSlot *consumerPack)   {
    int tempID = consumerPack->paketID;
    int vorgaengerID = (consumerPack->vorgaenger)->paketID;
    int nachfolgerID = (consumerPack->nachfolger)->paketID;
    
    if(vorgaengerID == -1) {
        vorgaengerID = 101;
    }

    tupel tempTupel = {vorgaengerID,tempID,nachfolgerID};

    pthread_mutex_lock(&queueMutex[tempID]);
    pthread_mutex_lock(&queueMutex[vorgaengerID]);
    pthread_mutex_lock(&queueMutex[nachfolgerID]);

    return tempTupel;
}

tupel lockElementsProd(qeueSlot *tempSlot, int tempID)   {
    int nachfolgerID = tempSlot->paketID;
    int vorgaengerID = (tempSlot->vorgaenger)->paketID;

    if(vorgaengerID == -1) {
        vorgaengerID = 101;
    }

    tupel tempTupel = {vorgaengerID, tempID, nachfolgerID};

    pthread_mutex_lock(&queueMutex[nachfolgerID]);
    pthread_mutex_lock(&queueMutex[vorgaengerID]);

    return tempTupel;
}


void unlockElements(tupel tempTupel)   {
    pthread_mutex_unlock(&queueMutex[tempTupel.mittlereID]);
    pthread_mutex_unlock(&queueMutex[tempTupel.vorgaengerID]);
    pthread_mutex_unlock(&queueMutex[tempTupel.nachfolgerID]);
}