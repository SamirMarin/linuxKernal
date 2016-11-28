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
    unsigned long * sp = (unsigned long*) process->sp;
    sp--;
    int state = process->state;
    if (state == STATE_WAITING) {
        *sp = -2;
    } else if (state == STATE_RECV || state == STATE_SEND || state == STATE_SLEEP) {
        *sp = -362;
    } else {
        *sp = process->rc;
    }
    unsigned long originalMask = process->signalBitMask;
    unsigned long one = 1;
    process->signalBitMask = originalMask | (one << sig_no);
    removeNthPCB(process);
    ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
    return 0;
}

void sigtramp(void (*handler)(void*), void *cntx) {
    handler(cntx);
    syssigreturn(cntx);
}
