/* create.c : create a process
*/

#include <xeroskernel.h>
#include <i386.h>
#include <limits.h>

/* Your code goes here. */

// Needed to prevent overflow of the PIDs
#define MAX_REUSE_COUNT ((INT_MAX - 32) / 32)
#define ENABLE_INTERRUPTS 0x3200

int create(void (*func)(void), int stackSize);
int nextPid(int reused_count, int index);
int createIdle(void (*func)(void), int stackSize);
struct pcb * setup_pcb(void (*func)(void), int stackSize);
void initFDT( struct FD *fd);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  create
 *  Description:  wrapper that creates regular process
 *  return     :  PID of process if successful or 0 if not
 * =====================================================================================
 */
int create(void (*func)(void), int stackSize) {
    struct pcb * process = setup_pcb(func, stackSize);
    if (!process) {
        return 0;
    }
    process->pid = nextPid(process->reuseCount, process->index);
    ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
    return process->pid;
}
//TODO
//TODO: remove function I don't think we need it anymore we can call idle first and use index starting from zero this way
//idle will always be assigned a zero.
int createIdle(void (*func)(void), int stackSize) {
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  createIdle
     *  Description:  wrapper that creates idle process
     *  Return     :  PID of idle = 0 if successful, -1 if not sucessful
     * =====================================================================================
     */
    struct pcb * process = setup_pcb(func, stackSize);
    if (!process) {
        return -1;
    }
    process->pid = 0;
    ready(process, &idleProcessHead, &idleProcessTail, STATE_READY);
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  setup_pcb
 *  Description:  allocates an unused process control block and returns a pointer to the
 *                process control block
 *       return:  a pointer to a process control block if successful or NULL if unsuccessful.
 * =====================================================================================
 */
struct pcb * setup_pcb(void (*func)(void), int stackSize) {
    struct pcb* newPcb = next(&stopQueueHead, &stopQueueTail);
    if (!newPcb) {
        kprintf("stopQueue has no available pcb function: create, file: create.c ");
        return NULL;
    }
    // Top of the stack (lowest address);
    unsigned long * stack = kmalloc(stackSize);
    if (!stack) {
        kprintf("kmalloc returned a null pointer function: create, file: create.c ");
        return NULL;
    }

    unsigned long topStack = (unsigned long) stack;

    unsigned long sizeStackAligned = (stackSize & 0xfffffff0); // don't add 16 since that is the header
    // kprintf("\n\n Top of stack: %d", topStack);
    // Bottom of the stack (highest address)
    unsigned long bottomStack = (unsigned long) (topStack + sizeStackAligned);
    //kprintf("\n\n Bottom of stack: %d", bottomStack);
    unsigned long safetyMargin = (unsigned long) sizeof(unsigned long);
    // Allocate space for the PCB
    unsigned long cpuStatePointer = (unsigned long) (bottomStack - sizeof(struct CPU) - safetyMargin);
    unsigned long *return_address = (unsigned long*) (bottomStack - safetyMargin);
    // Setup default signal to be ignore for each function
    int i;
    for (i = 0; i < SIGNALMAX; i++) {
            newPcb->sigFunctions[i] = NULL;
    }
    initFDT(newPcb->FDT);
    *return_address = (unsigned long) &sysstop;
    newPcb->memoryStart = (unsigned long*) topStack;
    newPcb->cpuState = (struct CPU*) cpuStatePointer;
    newPcb->sp = (unsigned long) cpuStatePointer;
    newPcb->cpuState->edi = 0;
    newPcb->cpuState->esi = 0;
    newPcb->cpuState->ebp = 0;
    newPcb->cpuState->esp = 0;
    newPcb->cpuState->ebx = 0;
    newPcb->cpuState->edx = 0;
    newPcb->cpuState->ecx = 0;
    newPcb->cpuState->eax = 0;
    newPcb->cpuState->iret_eip = (unsigned long) (func);
    newPcb->cpuState->iret_cs = getCS();
    newPcb->cpuState->eflags = ENABLE_INTERRUPTS;
    newPcb->reuseCount += 1;
    newPcb->signalBitMask = 0;
    if (newPcb->reuseCount > MAX_REUSE_COUNT) {
        newPcb->reuseCount = 1;
    }
    return newPcb;
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  nextPid
 *  Description:  obtains a process id for the created process
 *       Return:  the next available process id.
 * =====================================================================================
 */
int nextPid(int reuseCount, int index){
    return (PCBTABLESIZE * reuseCount) + index;
}
void initFDT( struct FD *fd){
    int i;
    for(i = 0; i < FDTSIZE; i++){
        fd[i].index = i;
        fd[i].status = 0;
        fd[i].majorNum = -1;
    }
}
