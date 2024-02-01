#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>  
#include <pthread.h>

#define QMAX 25
//Anzahl momentaner Packages in Qeue

extern pthread_mutex_t queueMutex[102];
extern pthread_mutex_t lockMutex;
extern pthread_mutex_t unlockMutex;
extern int packInQeue;
extern int consumedTotal;
extern int producedTotal;

extern pthread_cond_t mainProdCond;
extern pthread_cond_t mainConsCond;

//-1: no Stat; 1: min 1 Pack; 0: no Pack (empty) 
extern int qeueFilled;

//Elemente der Warteschlange werden hier in Warteslots referenziert 
typedef struct qeueSlot {
    struct qeueSlot *vorgaenger;
    struct qeueSlot *nachfolger;
    int paketID;
} qeueSlot;

int main(void);
int msleep(long msec);

#endif