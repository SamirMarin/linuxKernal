/* disp.c : dispatcher
*/

#include <xeroskernel.h>
#include <i386.h>
#include <stdarg.h>

struct pcb *readyQueueHead;
struct pcb *readyQueueTail;
struct pcb *recvAnyQueueHead;
struct pcb *recvAnyQueueTail;
struct pcb *stopQueueHead;
struct pcb *stopQueueTail;
struct pcb *idleProcessHead; 
struct pcb *idleProcessTail;


#define MAGIC_NUMBER 9999

extern long freemem; /* set in i386.c */
extern char * maxaddr;

void dispatch(void);
void cleanup(struct pcb *process);
struct pcb* next(struct pcb **head, struct pcb **tail);
int ready(struct pcb *process, struct pcb **head, struct pcb **tail, int state);
int killProcess(int pid, int currentPid);
void removeNthPCB(struct pcb *process);
void clearWaitingProcesses(struct pcb **head, struct pcb **tail, int retCode);
void testCleanup(void);
void setupSignal(struct pcb *process);
int registerHandler(int signal, void(*newHandler)(void *), void(**oldHandler)(void*), struct pcb *pcb);
void wait(int pid, struct pcb *p);
struct pcb* runIdleIfReadyEmpty(struct pcb **head);
int getCPUtimes(struct pcb *p, struct processStatuses *ps);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dispatch
 *  Description:  process systems call and schedules the next process, if given non existent
 *                system call prints error message and loops forever.
 * =====================================================================================
 */
void dispatch(void) {
    struct pcb *process = next(&readyQueueHead, &readyQueueTail);

    while (1) {
        setupSignal(process);
        int request = contextswitch(process);

        switch( request ) {
            case(CREATE): 
                {
                    void (*func)(void) = (void (*)(void)) *(process->args + 1);
                    int stack  = (int) *(process->args + 2);
                    int res = create(func, stack);
                    process->rc = res;
                    break;
                }
            case(TIMER_INT):
                {
                    tick();
                    process->cpuTime++;
                    ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
                    process = next(&readyQueueHead, &readyQueueTail);
                    end_of_intr();
                    break;
                }
            case(YIELD): 
                {
                    ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(STOP): 
                {
                    cleanup(process);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(GETPID): 
                {
                    process->rc = process->pid;
                    break;
                }
            case(PUTS): 
                {
                    char * str = (char *) *(process->args + 1);
                    kprintf(str);
                    break;
                }
            case(KILL): 
                {
                    int pid = (int) *(process->args + 1);
                    int sig_no = (int) *(process->args + 2);
                    // Old kill code, need to change after A3 is completed
                    //process->rc = killProcess(pid, process->pid);
                    process->rc = signal(pid, sig_no);
                    ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(SEND):
                {
                    //call the send in msg.c
                    int pid = (int) *(process->args + 1);
                    unsigned long num = (unsigned long) *(process->args + 2);
                    process->rc = send(pid, num, process);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;

                }
            case(RECEIVE):
                {
                    unsigned int * from_pid = (unsigned int *) *(process->args + 1);
                    unsigned long * num = (unsigned long *) *(process->args + 2);
                    process->rc = recv(from_pid, num, process);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(SLEEP):
                {
                    unsigned int ms = (unsigned int) *(process->args + 1);
                    sleep(ms, process);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(SIG_HANDLER):
                {
                    int sig_no = (int) *(process->args + 1);
                    void (*handler)(void*) = (void (*)(void*)) *(process->args + 2);
                    void (**oldHandler)(void*) = (void (**)(void*)) *(process->args + 3);
                    process->rc = registerHandler(sig_no, handler, oldHandler, process);
                    ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(SIG_RETURN):
                {
                    unsigned long *oldSP = (unsigned long *) *(process->args + 1);
                    int retCode = (int) *(oldSP - 1);
                    process->rc = retCode;
                    process->sp = (unsigned long) oldSP;
                    break;
                }
            case(WAIT):
                {
                    int pid = (int) *(process->args + 1);
                    wait(pid, process);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(CPU_TIMES):
                {
                    struct processStatuses *ps = (struct processStatuses*) *(process->args + 1);
                    process->rc = getCPUtimes(process, ps);
                    break;
                }
            case(OPEN):
                {
                    int device_no = (int) *(process->args + 1);
                    process->rc = di_open(process, device_no);
                    break;
                }
            case(CLOSE):
                {
                    int fd = (int) *(process->args + 1);
                    process->rc = di_close(process, fd);
                    break;
                }
            case(WRITE):
                {
                    int fd = (int) *(process->args + 1);
                    void *buff = (void*) *(process->args + 2);
                    int bufflen = (int) *(process->args + 3);
                    process->rc = di_write(process, fd, buff, bufflen);
                    break;
                }
            case(READ):
                {
                    int fd = (int) *(process->args + 1);
                    void *buff = (void*) *(process->args + 2);
                    int bufflen = (int) *(process->args + 3);
                    di_read(process, fd, buff, bufflen);
                    process = next(&readyQueueHead, &readyQueueTail);
                    break;
                }
            case(IOCTL):
                {
                    int fd = (int) *(process->args + 1);
                    unsigned long command = (unsigned long) *(process->args + 2);
                    int val = (int) *(process->args + 3);
                    process->rc = di_ioctl(process, fd, command, val);
                    break;
                }
            case (KEYBOARD):
                {
                    kbd_read_in();
                    end_of_intr();
                    break;
                }
            default:    
                {
                    kprintf("ERROR, request is: %d function: dispatch, file: disp.c", request);
                    for(;;);
                }
        }
    }
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  cleanup
 *  Description:  frees the stack for process and places the pcb on the stopped queue
 * =====================================================================================
 */
void cleanup(struct pcb *process) {
    //testCleanup();
    //kprintf("PID: %d, RETURNING MEMORY: %d\n", process->pid, process->memoryStart);
    process->pid = -1;
    int i;
    for(i = 0; i < FDTSIZE; i++){
        int mN = process->FDT[i].majorNum;
        if (mN > -1) {
            di_close(process, i);
        } 
    }
    kfree(process->memoryStart);
    clearWaitingProcesses(&(process->sendQHead), &(process->sendQTail), -1);
    clearWaitingProcesses(&(process->recvQHead), &(process->recvQTail), -1);
    clearWaitingProcesses(&(process->waitQHead), &(process->waitQTail), 0);
    ready(process, &stopQueueHead, &stopQueueTail, STATE_STOPPED);
    //testCleanup();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  clearWaitingProcesses
 *  Description:  removes process from the given head and tail and puts it on the ready queue with a return code that was passed in as retCode
 * =====================================================================================
 */
void clearWaitingProcesses(struct pcb **head, struct pcb **tail, int retCode) {
    while (*head && *tail) {
        struct pcb *process = next(head, tail);
        process->rc = retCode;
        ready(process, &readyQueueHead, &readyQueueTail, STATE_READY);
    }
    *head = NULL;
    *tail = NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  killProcess
 *  Description:  kills process of given pid returns 0, else returns -2 if pid == itself
 *                  else -1 if index or pid is invalid
 * =====================================================================================
 */
int killProcess(int pid, int currentPid){
    int index = (pid % PCBTABLESIZE);
    struct pcb* process = pcbTable + index;
    if (index < 0 || process->pid != pid) {
        return -1;
    }
    if(pid == currentPid){
        return -2;
    }
    removeNthPCB(process);
    cleanup(process);
    return 0;
}


void wait(int pid, struct pcb *p) {
    if (pid < 0) {
        p->rc = -1;
        ready(p, &readyQueueHead, &readyQueueTail, STATE_READY);
    }
    int index = (pid % PCBTABLESIZE);
    struct pcb* process = pcbTable + index;
    if (index < 0 || process->pid != pid) {
        p->rc = -1;
        ready(p, &readyQueueHead, &readyQueueTail, STATE_READY);
    }
    p->state = STATE_WAITING;
    ready(p, &(process->waitQHead), &(process->waitQTail), STATE_WAITING);


}

// This function is the system side of the sysgetcputimes call.
// It places into a the structure being pointed to information about
// each currently active process.
//  p - a pointer into the pcbtab of the currently active process
//  ps  - a pointer to a processStatuses structure that is
//        filled with information about all the processes currently in the system
//

  
int getCPUtimes(struct pcb *p, struct processStatuses *ps) {
  
  int i, currentSlot;
  currentSlot = -1;

  // Check if address is in the hole
  if (((unsigned long) ps) >= HOLESTART && ((unsigned long) ps <= HOLEEND)) {
    return -1;
  }

  //Check if address of the data structure is beyone the end of main memory
  if ((((char * ) ps) + sizeof(struct processStatuses)) > maxaddr)  {
    return -2;
  }

  // There are probably other address checks that can be done, but this is OK for now


  for (i=0; i < PCBTABLESIZE; i++) {
    struct pcb *currentProcess = &pcbTable[i];
    if (currentProcess->state != STATE_STOPPED) {
      // fill in the table entry
      currentSlot++;
      ps->pid[currentSlot] = currentProcess->pid;
      ps->status[currentSlot] = p == currentProcess ? STATE_RUNNING : currentProcess->state;
      ps->cpuTime[currentSlot] = currentProcess->cpuTime * TICKLENGTH;
    }
  }

  return currentSlot;
}


int registerHandler(int signal, void(*newHandler)(void *), void(**oldHandler)(void*), struct pcb *p) {
    if (signal < 0 || signal > SIGNALMAX) {
        return -1;
    }
    if (((char *) newHandler) > maxaddr) {
        return -2;
    }
    if (((unsigned long) newHandler) > HOLESTART && ((unsigned long) newHandler) < HOLEEND) {
        return -2;
    }

    if (oldHandler) {
        *oldHandler = p->sigFunctions[signal];
    }
    p->sigFunctions[signal] = newHandler;

    return 0;
}

void setupSignal(struct pcb* process) {
    if (!process->signalBitMask) {
            return;
    }
    unsigned long sigBM = process->signalBitMask;
    int signalNo = 0;
    // Determine largest signal number to process
    while (sigBM >>= 1) {
        signalNo++;
    }

    void (*handler)(void*) = process->sigFunctions[signalNo];
    // Handler is null, so we ignore signal
    if (!handler) {
        // Set the bit in the signal to be zero
        unsigned long one = 1;
        unsigned long newSignalBitMask = process->signalBitMask;
        process->signalBitMask = newSignalBitMask & ~(one << signalNo);
        return;
    }

    // PREPARE ARGUMENTS FOR SIGTRAMP
    unsigned long * sp = (unsigned long *) process->sp;
    sp--;
    *sp = process->rc;
    sp--;
    *sp = process->sp; // old context
    sp--;
    *sp = (unsigned long) handler; //handler function
    sp--;
    // Instead of return address (for testing)
    *sp = MAGIC_NUMBER;


    // SETUP NEW CONTEXT
    struct CPU* context = (struct CPU*) sp;

    // Move pointer down so that we can fit the CPU State
    context--;

    context->edi = 0;
    context->esi = 0;
    context->ebp = 0;
    context->esp = 0;
    context->ebx = 0;
    context->edx = 0;
    context->ecx = 0;
    context->eax = 0;
    context->iret_eip = (unsigned long) (&sigtramp);
    context->iret_cs = getCS();
    context->eflags = 0x3200;

    // Set up process stack pointer to look like it begins where the new context is;
    process->sp = (unsigned long) context;

    // Set the bit in the signal as delivered
    unsigned long one = 1;
    unsigned long newSignalBitMask = process->signalBitMask;
    process->signalBitMask = newSignalBitMask & ~(one << signalNo);

    return;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  removeNthPCB
 *  Description:  removes the given process from what ever queue it is currently in.
 * =====================================================================================
 */
void removeNthPCB(struct pcb *process){
    //process is head and tail
    if(!(process->prev) && !(process->next)){
        *(process->head) = NULL;
        *(process->tail) = NULL;
    }
    //process is the head
    else if(!(process->prev)){
        struct pcb *temp = process->next;
        *(process->head) = temp;
        temp->prev = NULL;
        process->next = NULL;
    }
    //process is the tail
    else if(!(process->next)){
        struct pcb *temp = process->prev;
        *(process->tail) = temp;
        temp->next = NULL;
        process->prev = NULL;
    }
    //process not head or tail, in middle of queue
    else{
        struct pcb *tempPrev = process->prev;
        struct pcb *tempNext = process->next;
        tempPrev->next = tempNext;
        tempNext->prev = tempPrev;
        process->next = NULL;
        process->prev = NULL;
    }
    process->head = NULL;
    process->tail = NULL;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ready
 *  Description:  adds a pcb process to given list identified by head and tail.
 *       Return:  1 if sucessful 0 if unsucessful
 * =====================================================================================
 */
int ready(struct pcb *process, struct pcb **head, struct pcb **tail, int state){
    if(!process){
        return -1;
    }
    if(process == idleProcessHead){
        return -2;
    }
    if(!*head && !*tail){
        *head = process;
        *tail = process;
        process->prev = NULL;
        process->next = NULL;
        process->head = head;
        process->tail = tail;
        process->state = state;
        return 1;
    }
    if(*head && *tail){
        (*tail)->next = process;
        process->prev = *tail;
        *tail = process;
        process->head = head;
        process->tail = tail;
        (*tail)->next = NULL;
        process->state = state;
        return 1;
    }
    kprintf("\n\n one of QueueHead or QueueTail is NULL\n file: disp.c\n function: ready");
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  next
 *  Description:  removes a pcb struct from given list identified by the given head.
 *       Return:  return pcb struct, null if non availble in given list
 * =====================================================================================
 */
struct pcb* next(struct pcb **head, struct pcb **tail){
    if(!*head){
        // gets idle process only if dealing with ready queue
        struct pcb* idleProcess = runIdleIfReadyEmpty(head);
        return idleProcess;
    }
    //if they are the same only one thing on list
    if (*head == *tail) {
        *tail = NULL;
    }
    struct pcb *nextProcess = *head;
    *head = (*head)->next;
    nextProcess->next = NULL;
    nextProcess->prev = NULL;
    (*head)->prev = NULL;
    nextProcess->head = NULL;
    nextProcess->tail = NULL;
    return nextProcess;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  runIdleIfReadyEmpty
 *  Description:  if readyQueue is empty runs the idle prcess
 *      Return :  pcb  * to the idleProcessHead if readyQueue Empty, else NULL
 * =====================================================================================
 */
struct pcb* runIdleIfReadyEmpty(struct pcb **head){
    if(head == &readyQueueHead){
        return idleProcessHead;
    }
    return NULL;
}

//Test function for cleanup
void testCleanup(void){

    struct pcb *sqht = stopQueueHead;
    int i = 1;
    while(sqht){
        //kprintf("(sqht: %d)-->", sqht->state);
        sqht = sqht->next;
        i++;
    }
    kprintf("\nThere are %d, processes in the stopped queue", i);
}
