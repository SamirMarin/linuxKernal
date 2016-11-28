/* sleep.c : sleep device 
   This file does not need to modified until assignment 2
   */

#include <xeroskernel.h>
#include <xeroslib.h>
struct pcb *sleepQueueHead; //points to sleep queue head
struct pcb *sleepQueueTail; //points to sleep queue tail 

int insertIntoSleepQ(struct pcb * process, unsigned int tick);
unsigned int sleep(unsigned int ms, struct pcb * process);
void tick(void);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sleep
 *  Description:  adds process to the the sleep queue if ms > 0
 * =====================================================================================
 */
unsigned int sleep(unsigned int ms, struct pcb * process){
    if(!ms){
        process->rc = 0;
        ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
        return 0;
    }
    unsigned int tick = ms/TICKLENGTH + (ms%TICKLENGTH ? 1 : 0);
    process->state = STATE_SLEEP;
    insertIntoSleepQ(process, tick); 
    return 1;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  tick
 *  Description:  subtracts one tick off the head of the sleep queue, and adds process that have tick <= 0 to ready Queue
 * =====================================================================================
 */
void tick(void){
    if (sleepQueueHead) {
        sleepQueueHead->tick -= 1;
    }
    while (sleepQueueHead && sleepQueueHead->tick <= 0) {
        struct pcb *process = next(&sleepQueueHead, &sleepQueueTail);
        process->rc = 0;
        ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
    }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  insertIntoSleepQ
 *  Description:  inserts into sleep queue following delta list convention
 * =====================================================================================
 */
int insertIntoSleepQ(struct pcb * process, unsigned int tick){
    if(!sleepQueueTail && !sleepQueueHead){
        sleepQueueHead = process;
        sleepQueueTail = process;
        process->tick = tick;
        process->head = &sleepQueueHead;
        process->tail = &sleepQueueTail;
        return 1;
    }

    if(sleepQueueHead && sleepQueueTail){
        struct pcb *current = sleepQueueHead;
        struct pcb *prev = NULL;
        while(current && tick >= current->tick){
            tick -= current->tick;
            prev = current;
            current = current->next;
        }
        process->tick = tick;

        if(!prev){
            sleepQueueHead = process;
        } else {
            prev->next = process;
        }
        if (current) {
            current->prev = process;
            current->tick -= tick;
        } else {
            sleepQueueTail = process;
        }

        process->next = current;
        process->prev = prev;
        process->head = &sleepQueueHead;
        process->tail = &sleepQueueTail;
        return 1;
    }
    kprintf("\n\n one of sleepQueueHead or sleepQueueTail is NULL\n file: sleep.c\n function: insert");
    return 0;
}

