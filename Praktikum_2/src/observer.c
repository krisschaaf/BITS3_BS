#include "observer.h"

//Der Aufruf nach jeder Sekunde erfolgt außerhalb dieses Moduls
void *observe(void *firstSlot)
{
    while (consumedTotal < 1000)
    {
        pthread_mutex_lock(&queueMutex[0]);
        qeueSlot *tempSlot = firstSlot;
        
        tempSlot = tempSlot->nachfolger;       
        pthread_mutex_unlock(&queueMutex[0]);
        pthread_mutex_lock(&queueMutex[tempSlot->paketID+1]);

        if (packInQeue == 0 || tempSlot->paketID == 100)  //###### != 100 -> == 100
        {
            printf("Packages in Queue: <Empty>\nConsumed Packages: %d\n", consumedTotal);
        }
        else
        {
            printf("Packages in Queue: (%d) ", packInQeue);
            
            //Array mit allen Qeue Paketen füllen
            while (tempSlot->paketID != 100)
            {
                
                printf("%d ", tempSlot->paketID);
                tempSlot = (tempSlot->nachfolger);
                
                pthread_mutex_unlock(&queueMutex[tempSlot->vorgaenger->paketID+1]);
                pthread_mutex_lock(&queueMutex[tempSlot->paketID+1]);
            }
            
            printf("\n");

            printf("Consumed Packages: %d\n", consumedTotal);
        }
        pthread_mutex_unlock(&queueMutex[tempSlot->paketID+1]);
        msleep(1000);
    }
    printf("Ende: Alle 1000 Arbeitspakete wurden verarbeitet\n");

    return NULL;
}