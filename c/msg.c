/* msg.c : messaging system 
   This file does not need to modified until assignment 2
   */

#include <xeroskernel.h>
#include <stdarg.h>


int send(int dest_pid, unsigned long num, struct pcb * currentProcess);
int recv(unsigned int *from_pid, unsigned long *num, struct pcb * currentProcess);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  send
 *  Description:  sends num to dest_pid process returs 0 is successful, else returns -1 if invalid pid or index out of bounds
 *                  -2 if sending to itself, and -3 if any other errors
 * =====================================================================================
 */
int send(int dest_pid, unsigned long num, struct pcb * currentProcess) {
    if (dest_pid < 1) {
        ready(currentProcess, &readyQueueHead, &readyQueueTail);
        return -1;
    }

    int index = (dest_pid % PCBTABLESIZE) - 1;
    struct pcb* foundProcess = pcbTable + index;

    if (index < 0 || foundProcess->pid != dest_pid) {
        ready(currentProcess, &readyQueueHead, &readyQueueTail);
        return -1;
    }

    if (dest_pid == currentProcess->pid) {
        ready(currentProcess, &readyQueueHead, &readyQueueTail);
        return -2;
    }

    // check that the foundProcess is in the queue of receivers for the currentProcess
    // check that the foundProcess is in the queue of receivers who are receiving any
    if (foundProcess->head == &(currentProcess->recvQHead) || foundProcess->head == &recvAnyQueueHead) {
        // put the value of num in the recv's address
        unsigned int * from_pid = (unsigned int *) *(foundProcess->args + 1);
        unsigned long * dest_num = (unsigned long *) *(foundProcess->args + 2);
        if ((*from_pid != currentProcess->pid) && *from_pid) {
            ready(currentProcess, &readyQueueHead, &readyQueueTail);
            return -3;
        }

        *dest_num = num;
        *from_pid = currentProcess->pid;
        // remove foundProcess from the queue it was in
        removeNthPCB(foundProcess);
        // put both processes on the ready queue
        ready(currentProcess, &readyQueueHead, &readyQueueTail);
        ready(foundProcess, &readyQueueHead, &readyQueueTail);

    } else {
        // put the currentprocess on the sender's queue of the process we found

        ready(currentProcess, &(foundProcess->sendQHead), &(foundProcess->sendQTail));
    }
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: recv
 *  Description:  recv num from from_pid process returs 0 is successful, else returns -1 if invalid pid or index out of bounds
 *                  -2 if receiving from itself, and -3 if any other errors
 * =====================================================================================
 */
int recv(unsigned int *from_pid, unsigned long *num, struct pcb * currentProcess) {
    // Non-zero PID
    if (*from_pid) {
        int index = (*from_pid % PCBTABLESIZE) - 1;
        struct pcb* foundProcess = pcbTable + index;
        if (index < 0 || foundProcess->pid != *from_pid) {
            ready(currentProcess, &readyQueueHead, &readyQueueTail);
            return -1;
        }
        if (*from_pid == currentProcess->pid) {
            ready(currentProcess, &readyQueueHead, &readyQueueTail);
            return -2;
        } 

        if ((unsigned long) (foundProcess->head) == (unsigned long) &(currentProcess->sendQHead)) {

            int dest_pid = (int) *(foundProcess->args + 1);
            unsigned long from_num = (unsigned long) *(foundProcess->args + 2);
            if (dest_pid != currentProcess->pid) {
                ready(currentProcess, &readyQueueHead, &readyQueueTail);
                return -3;
            }

            *num = from_num;
            removeNthPCB(foundProcess);
            ready(currentProcess, &readyQueueHead, &readyQueueTail);
            ready(foundProcess, &readyQueueHead, &readyQueueTail);
        } else {
            ready(currentProcess, &(foundProcess->recvQHead), &(foundProcess->recvQTail));
        }
    } else {
        // from_pid is zero.
        // check the sender's queue of the current Process and take the first thing from there
        struct pcb *foundProcess = next(&(currentProcess->sendQHead), &(currentProcess->sendQTail));
        if (!foundProcess) {
            ready(currentProcess, &recvAnyQueueHead, &recvAnyQueueTail);
            return 0;
        }

        int dest_pid = (int) *(foundProcess->args + 1);
        unsigned long from_num = (unsigned long) *(foundProcess->args + 2);

        if (dest_pid != currentProcess->pid) {
            return -3;
        }
        *num = from_num;
        *from_pid = foundProcess->pid; 
        ready(currentProcess, &readyQueueHead, &readyQueueTail);
        ready(foundProcess, &readyQueueHead, &readyQueueTail);

    }
    return 0;

}
