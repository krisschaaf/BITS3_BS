/**
 * @file vmaccess.c
 * @author Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * @date 2010
 * @brief The access functions to virtual memory.
 */

#include "vmaccess.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#include "syncdataexchange.h"
#include "vmem.h"
#include "debug.h"
#include "error.h"

/*
 * static variables
 */

static struct vmem_struct *vmem = NULL; //!< Reference to virtual memory

/**
 * The progression of time is simulated by the counter g_count, which is incremented by 
 * vmaccess on each memory access. The memory manager will be informed by a command, whenever 
 * a fixed period of time has passed. Hence the memory manager must be informed, whenever 
 * g_count % TIME_WINDOW == 0. 
 * Based on this information, memory manager will update aging information
 */

static int g_count = 0;    //!< global acces counter as quasi-timestamp - will be increment by each memory access
#define TIME_WINDOW   20

/**
 *****************************************************************************************
 *  @brief      This function setup the connection to virtual memory.
 *              The virtual memory has to be created by mmanage.c module.
 *
 *  @return     void
 ****************************************************************************************/
static void vmem_init(void) {

    vmem = vmem;
    //TODO vmem_init(void) aus mmanage.c benutzen!

    /* Create System V shared memory */

    /* We are only using the shm, don't set the IPC_CREAT flag */

    /* attach shared memory to vmem */

}

/**
 *****************************************************************************************
 *  @brief      This function puts a page into memory (if required). Ref Bit of page table
 *              entry will be updated.
 *              If the time window handle by g_count has reached, the window window message
 *              will be send to the memory manager. 
 *              To keep conform with this log files, g_count must be increased before 
 *              the time window will be checked.
 *              vmem_read and vmem_write call this function.
 *
 *  @param      address The page that stores the contents of this address will be 
 *              put in (if required).
 * 
 *  @return     void
 ****************************************************************************************/
static void vmem_put_page_into_mem(int address) {
    //TODO Seite mit param adress aus 
    //TODO Seite in mainMemory schreiben
    //TODO Ref Bit dieser Seite setzen
    //
}

/**
 *****************************************************************************************
 *  @brief      This function reads an integer value from virtual memory.
 *              If this functions access virtual memory for the first time, the 
 *              virtual memory will be setup and initialized.
 *
 *  @param      address The virtual memory address the integer value should be read from.
 * 
 *  @return     The int value read from virtual memory.
 ****************************************************************************************/
int vmem_read(int address) {
    // Adresse kommt vom anderem Modul und ist die  -> daraus virtuelle Seitennummer berechnen und in ptentry schreiben
    //TODO berechnet aus der virtuellen Speicheradresse die Frame-Nummer und den Offset 
    //TODO liest Seiten aus mainMemory -> wenn Seite fehlt wird blockiert und der mmanage.c wird über PageFault benachrichtigt 
}

/**
 *****************************************************************************************
 *  @brief      This function writes an integer value from virtual memory.
 *              If this functions access virtual memory for the first time, the 
 *              virtual memory will be setup and initialized.
 *
 *  @param      address The virtual memory address the integer value should be written to.
 *
 *  @param      data The integer value that should be written to virtual memory.
 * 
 *  @return     void
 ****************************************************************************************/
void vmem_write(int address, int data) {
        //TODO berechnet aus der virtuellen Speicheradresse die Frame-Nummer und den Offset
        //TODO schreibt Seiten in mainMemory -> bei PageFault (kein Speicher mehr im mainMemory) wird mmanage.c benachrichtigt
}
// EOF
