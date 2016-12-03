/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>


int signal(int pid, int sig_no);
void sigtramp(void (*handler)(void*), void *cntx);

int signal(int pid, int sig_no) {
    int index = (pid % PCBTABLESIZE);
    struct pcb* process = pcbTable + index;
    if (index < 0 || process->pid != pid || pid < 0) {
        return -1;
    }
    if (sig_no < 0 || sig_no > SIGNALMAX) {
        return -2;
    }

    int state = process->state;
    if (state == STATE_WAITING) {
        removeNthPCB(process);
        ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
        process->rc = -2;
    }
    if (state == STATE_RECV || state == STATE_SEND || state == STATE_SLEEP)  {
        removeNthPCB(process);
        ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
        process->rc  = -362;
    }
    if (state == STATE_DEV_WAITING) { 
        process->rc  = -362;
    }
    unsigned long originalMask = process->signalBitMask;
    unsigned long one = 1;
    process->signalBitMask = originalMask | (one << sig_no);
    return 0;
}

void sigtramp(void (*handler)(void*), void *cntx) {
    handler(cntx);
    syssigreturn(cntx);
}
