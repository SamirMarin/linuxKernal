/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>


int signal(int pid, int sig_no);

int signal(int pid, int sig_no) {
    int index = (pid % PCBTABLESIZE) - 1;
    struct pcb* process = pcbTable + index;
    if (index < 0 || process->pid != pid) {
        return -1;
    }
    if (sig_no < 0 || sig_no > 31) {
        return -2;
    }

    // TODO NEED TO HANDLE CASE WHEN PROCESS WAS BLOCKED

    int originalMask = process->signalBitMask;
    long one = 1;
    process->signalBitMask = originalMask | (one << sig_no);

    return 0;
}
