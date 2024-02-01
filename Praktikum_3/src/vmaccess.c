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

static int g_count = 0; //!< global acces counter as quasi-timestamp - will be increment by each memory access
#define TIME_WINDOW 20

/**
 *****************************************************************************************
 *  @brief      This function setup the connection to virtual memory.
 *              The virtual memory has to be created by mmanage.c module.
 *
 *  @return     void
 ****************************************************************************************/
static void vmem_init(void)
{

    /* Create System V shared memory */

    /* We are only using the shm, don't set the IPC_CREAT flag */
    key_t key = ftok(SHMKEY, SHMPROCID);

    int id = shmget(key, SHMSIZE, 0);

    /* attach shared memory to vmem */

    vmem = shmat(id, NULL, 0);
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
static void vmem_put_page_into_mem(int address)
{

    // Ermittlung der Seitennummer in vmem_read beschrieben
    int virtuelleSeitennummer = address / VMEM_PAGESIZE;

    // Senden der Pagefault Nachricht, wenn Present Flag nicht gesetzt ist
    if ((vmem->pt[virtuelleSeitennummer].flags & PTF_PRESENT) == 0)
    {
        struct msg pageFaultMsg = {CMD_PAGEFAULT, virtuelleSeitennummer, g_count, 0};
        sendMsgToMmanager(pageFaultMsg);
    }
    else
    {
        // bitweises verodern der Flags mit dem Reference Bit -> Seite wurde verwendet
        // Nur für den Clock Algorithmus relevant
        vmem->pt[virtuelleSeitennummer].flags |= PTF_REF;
        g_count++;
    }

    // senden der Window Message an den Memory Manager
    // Hence the memory manager must be informed, whenever g_count % TIME_WINDOW == 0.
    if ((g_count % TIME_WINDOW) == 0)
    {
        struct msg timeOutMsg = {CMD_TIME_INTER_VAL, 0, g_count, 0};
        sendMsgToMmanager(timeOutMsg);
    }
}

/*In dieser Funktion wird eine virtuelle Adresse auf eine reale Adresse abgebildet
Virtuelle Adresse = Virtuelle Seitennummer * Seitengröße + Offset */
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
int vmem_read(int address)
{
    // Initialisierung des Virtual Memories, falls auf diesen das erste Mal zugegriffen wird
    if (vmem == NULL)
    {
        vmem_init();
    }

    vmem_put_page_into_mem(address);

    // Ermittlung der virtuellen Seitenummer:
    // Virtuelle Seitennummer = floor (virtuelle Adresse / Seitengröße)
    int virtuelleSeitennummer = address / VMEM_PAGESIZE;

    // Ermittlung des Offsets:
    //  Offset = virtuelle Adresse modulo Seitengröße
    //  "Positionsnummer" innerhalb der Seite
    int offset = address % VMEM_PAGESIZE;

    // Ermittlung der Seitenrahmennummer durch Verwendung einer Seitentabelle
    //  Index = virtuelle Seitennummer
    int seitenrahmennummer = vmem->pt[virtuelleSeitennummer].frame;

    // Integer welcher an übergebener Adresse im Speicher steht
    return vmem->mainMemory[seitenrahmennummer * VMEM_PAGESIZE + offset];
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
void vmem_write(int address, int data)
{
    // Initialisierung des Virtual Memories, falls auf diesen das erste Mal zugegriffen wird
    if (vmem == NULL)
    {
        vmem_init();
    }

    vmem_put_page_into_mem(address);

    // Ermittlung der Nummern und Adressen in obiger Funktion beschrieben
    int virtuelleSeitennummer = address / VMEM_PAGESIZE;
    int offset = address % VMEM_PAGESIZE;

    // Setzen des Dirty Flags da in Seite geschrieben wurde
    vmem->pt[virtuelleSeitennummer].flags |= PTF_DIRTY;

    // Seitenrahmennummer in welche der Integer geschrieben werden soll
    int seitenrahmennummer = vmem->pt[virtuelleSeitennummer].frame;

    //Daten in jeweilige adresse schreiben
    vmem->mainMemory[seitenrahmennummer * VMEM_PAGESIZE + offset] = data;
}
// EOF
