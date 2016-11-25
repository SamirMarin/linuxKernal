/* user.c : User processes
*/

#include <xeroskernel.h>

/* Your code goes here */
void root(void);
void producer(void);
void consumer(void);
void testProcess(void);
void testYielder(void);
void testStopper(void);
void testKillProcess(void);
void testSend(void);
void testSend2(void);
void testSend3(void);
void testSend4(void);
void testSend0(void);
void testSendHelper(void);
void errorCheckingSend(int function, int result);
void errorCheckingRecv(int function, int result, unsigned long * mess);
void runSendToRecvTest(void);
void testRecv(void);
void testRecv2(void);
void testRecv3(void);
void testRecv4(void);
void testRecv0(void);
void testRecvHelper(void);
void runRecvToSendTest(void);
void runSendToRecvTes(void);

//global int for sending
unsigned int *globalPIDSend = 0;
unsigned int *globalPIDSend2 = 0;
unsigned int *globalPIDSend3 = 0;
unsigned int *globalPIDSend4 = 0;
int globalPidToReceive2 = 0;
//global pid for Reciving
int globalPIDRecv = 0;
int globalPIDRecv2 = 0;
int globalPIDRecv3 = 0;
int globalPIDRecv4 = 0;
int globalPIDRecv5 = 0;
unsigned int globalPIDToSend = 0;
int globalRoot = 0;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  root
 *  Description:  first process created by kernal
 * =====================================================================================
 */
void root(void) {
    int pid = sysgetpid();
    globalRoot = pid;
    kprintf("\nProcess %d is alive", pid);

    int pid1 = syscreate(&producer, 8000);
    kprintf("\nProcess %d has created process %d", pid, pid1);
    int pid2 = syscreate(&producer, 8000);
    kprintf("\nProcess %d has created process %d", pid, pid2);
    int pid3 = syscreate(&producer, 8000);
    kprintf("\nProcess %d has created process %d", pid, pid3);
    int pid4 = syscreate(&producer, 8000);
    kprintf("\nProcess %d has created process %d", pid, pid4);
    syssleep(4000);
    syssend(pid3, 10000);
    syssend(pid2, 7000);
    syssend(pid1, 20000);
    syssend(pid4, 27000);
    unsigned long msg;
    unsigned int * p = (unsigned int *) &pid4;
    int res = sysrecv(p, &msg);
    kprintf("\nProcess %d has status from receive 4th process %d", pid, res);
    res = syssend(pid3, msg);
    kprintf("\nProcess %d has status from send to 3rd process %d", pid, res);
    sysstop();

    //testing functiong creation
    //syscreate(&testProcess, 7777);
    /* unsigned int fromAny = 0;
    //----Testing Inturrups-----
    unsigned long gettingBlocked = 6;
    sysrecv(&fromAny, &gettingBlocked);*/
    //---Testing Send to receive specific----
    //runSendToRecvTest();
    //runRecvToSendTest();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  producer
 *  Description:  producer function created by the root process for the extended
 *                producer/consumer problem
 * =====================================================================================
 */
void producer(void){
    int pid = sysgetpid();
    kprintf("\nProcess %d is alive    ", pid); // spaces are needed otherwise the print statements don't line up for some reason?
    syssleep(5000);
    unsigned int rootPid = globalRoot;
    unsigned long msg;
    sysrecv(&rootPid, &msg);
    kprintf("\nProcess %d has received the message and will sleep for %d", pid, msg);
    syssleep(msg);
    kprintf("\nProcess %d has stopped sleeping and is going to exit", pid);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  idleproc
 *  Description:  process that runs when no other process are available
 * =====================================================================================
 */
void idleproc( void ){
    for(;;){
        //kprintf("\nIm going idle not sure how to get out");
    }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  consumer
 *  Description:  consumer process called by root process
 * =====================================================================================
 */
void consumer(void){
    int i;
    for(i = 15; i > 0; i--){
        kprintf("Birthday UBC\n");
        sysyield();
    }
}

//test fuction for testing max process also starts test to syskill
void testProcess(void){
    int i;
    int currentProcess = 2;
    for(i = 30; i > 0; i--){
        int result = syscreate(&testYielder, 2000);
        kprintf("\ntesting create: %d create pid: %d ", currentProcess, result);
        currentProcess++;
    }
    syscreate(&testKillProcess, 2000);
    for(;;) {
        sysyield();
    }
}
//test the syskill
void testKillProcess(void){
    int i = 29;
    for (i = 29; i < 35; i++) {
        int result = syskill(i);
        kprintf("\ntesting kill: %d kill result: %d ", i, result);
    }

}
//test function for yielding process
void testYielder(void){
    for (;;) {
        sysyield();
    }
}
//test funtin for clean up creates 5 process and stops them
void testStop(void){
    int i;
    int currentProcess = 1;
    for(i = 5; i > 0; i--){
        int result = syscreate(&testStopper, 2000);
        kprintf("\ntesting create: %d create pid: %d ", currentProcess, result);
        currentProcess++;
    }
}
//test functin for stopping process
void testStopper(void){
    sysstop();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  runSendToRecvTest
 *  Description:  test senders to single receiver
 * =====================================================================================
 */
void runSendToRecvTest(void){
    int pid1 = syscreate(&testSend, 7777);
    int pid2 = syscreate(&testSend2, 7777);
    int pid3 = syscreate(&testSend3, 7777);
    int pid4 = syscreate(&testSend0, 8888);
    int pid5 = syscreate(&testSend4, 7777);
    globalPidToReceive2 = syscreate(&testSendHelper, 8888);
    kprintf("\ntestSend %d", pid1);
    kprintf("\ntestSend2 %d", pid2);
    kprintf("\ntestSend3 %d", pid3);
    kprintf("\ntestSend0 %d", pid4);
    kprintf("\ntestSend4 %d", pid5);
}

void testSend(void){
    unsigned long message = 3;
    int result = syssend(-100, message);
    if(!(result == -1 || result == -3)){
        kprintf("\ntesting send invalid, should have got -1 instead got: %d ", result);
        for(;;);
    }
    kprintf("\ntesting send invalid, sucess got: %d ", result);
    //testing sending to same process
    int pidToReceive = sysgetpid();
    globalPIDSend = (unsigned int*) &pidToReceive;
    kprintf("\nI'm the new globalPidSend: %d ", globalPIDSend);
    kprintf("\n I got the pid, its is %d", pidToReceive);
    result = syssend(pidToReceive, message);
    if(result != -2){
        kprintf("\ntesting send same, should have got -2 instead got: %d ", result);
        for(;;);
    }
    kprintf("\ntesting send same, success got: %d ", result);
    //    globalPidToReceive2 = syscreate(&testSendHelper, 8888);
    result = syssend(globalPidToReceive2, message);
    errorCheckingSend(1, result);
}
void testSend2(void){
    unsigned long message = 4;
    int pidToReceive = sysgetpid();
    kprintf("\n I got the pid, its is %d", pidToReceive);
    globalPIDSend2 = (unsigned int*) &pidToReceive;
    int result = syssend(globalPidToReceive2, message);
    errorCheckingSend(2, result);
}
void testSend3(void){
    unsigned long message = 5;
    int pidToReceive = sysgetpid();
    kprintf("\n I got the pid, its is %d", pidToReceive);
    globalPIDSend3 = (unsigned int*) &pidToReceive;
    int result = syssend(globalPidToReceive2, message);
    errorCheckingSend(3, result);
}
void testSend4(void){
    unsigned long message = 55;
    int pidToReceive = sysgetpid();
    kprintf("\n I got the pid, its is %d", pidToReceive);
    globalPIDSend4 = (unsigned int*) &pidToReceive;
    int result = syssend(globalPidToReceive2, message);
    errorCheckingSend(4, result);
}
void testSend0(void){
    unsigned long message = 6;
    int pidToReceive = sysgetpid();
    kprintf("\n I got the pid, its is %d", pidToReceive);
    int result = syssend(globalPidToReceive2, message);
    errorCheckingSend(0, result);
}
void errorCheckingSend(int function, int result){
    if(result){
        kprintf("\ntesting send%d failed, should have got 0 instead got: %d ", function, result);
        for(;;);
    }
    kprintf("\ntesting send%d sucess, got: %d ", function, result);
}
void errorCheckingRecv(int function, int result, unsigned long * mess){
    if(result){
        kprintf("\ntesting recv%d failed, should have got 0 instead got: %d ", function, result);
        for(;;);
    }
    kprintf("\ntesting recv%d sucess, got: %d the message is: %d ", function, result, *mess);
}
void testSendHelper(void){
    unsigned long theMessage = 100;
    unsigned long *message = &theMessage;
    kprintf("\n sedningPID:%d, message:%d", globalPIDSend, *message);
    int result = sysrecv(globalPIDSend, message);
    errorCheckingRecv(1, result, message);
    result = sysrecv(globalPIDSend2, message);
    errorCheckingRecv(2, result, message);
    result = sysrecv(globalPIDSend3, message);
    errorCheckingRecv(3, result, message);
    result = sysrecv(globalPIDSend4, message);
    errorCheckingRecv(4, result, message);
    unsigned int any = 0;
    result = sysrecv(&any, message);
    errorCheckingRecv(0, result, message);
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  runRecvToSendTest
 *  Description:  test recvers to single sender
 * =====================================================================================
 */
void runRecvToSendTest(void){
    syscreate(&testRecv, 7878);
    syscreate(&testRecv2, 7898);
    syscreate(&testRecv3, 8767);
    syscreate(&testRecv4, 8899);
    syscreate(&testRecv0, 8899);
    unsigned int pidFromHelper = syscreate(&testRecvHelper, 8888);
    globalPIDToSend = pidFromHelper;
}
void testRecv(void){
    unsigned long theMessage = 100;
    unsigned long *message = &theMessage;
    unsigned int *currentPid = 0;
    int result = sysrecv(currentPid, message);//should fail with 3
    if(!(result == -1 || result == -3)){
        kprintf("\ntesting recv invalid, should have got -1 instead got: %d ", result);
        for(;;);
    }
    kprintf("\ntesting recv invalid, sucess got: %d ", result);
    //testing receiving from same process
    int pidOfReciever = sysgetpid();
    globalPIDRecv = pidOfReciever;
    currentPid = (unsigned int*) &pidOfReciever;
    result = sysrecv(currentPid, message);
    if(result != -2){
        kprintf("\ntesting recv same, should have got -2 instead got: %d ", result);
        for(;;);
    }
    kprintf("\ntesting send recv, success got: %d ", result);
    result = sysrecv(&globalPIDToSend, message);
    errorCheckingRecv(1, result, message);

}
void testRecv2(void){
    unsigned long theMessage = 100;
    unsigned long *message = &theMessage;
    int pidOfReciever = sysgetpid();
    globalPIDRecv2 = pidOfReciever;
    int result = sysrecv(&globalPIDToSend, message);
    errorCheckingRecv(2, result, message);
}
void testRecv3(void){
    unsigned long theMessage = 100;
    unsigned long *message = &theMessage;
    int pidOfReciever = sysgetpid();
    globalPIDRecv3 = pidOfReciever;
    int result = sysrecv(&globalPIDToSend, message);
    errorCheckingRecv(3, result, message);
}
void testRecv4(void){
    unsigned long theMessage = 100;
    unsigned long *message = &theMessage;
    int pidOfReciever = sysgetpid();
    globalPIDRecv4 = pidOfReciever;
    int result = sysrecv(&globalPIDToSend, message);
    errorCheckingRecv(4, result, message);
}
void testRecv0(void){
    unsigned long theMessage = 100;
    unsigned long *message = &theMessage;
    int pidOfReciever = sysgetpid();
    globalPIDRecv5 = pidOfReciever;
    unsigned int *any= 0;
    unsigned int receiveFromAny = 0;
    any = &receiveFromAny;
    int result = sysrecv(any, message);
    errorCheckingRecv(0, result, message);
    result = sysrecv(any, message);
    *any = 0;
    errorCheckingRecv(10, result, message);
    result = sysrecv(any, message);
    *any = 0;
    errorCheckingRecv(20, result, message);
}
void testRecvHelper(void){
    unsigned long message = 500;
    int result = syssend(globalPIDRecv, message);
    errorCheckingSend(1, result);
    result = syssend(globalPIDRecv2, message);
    errorCheckingSend(2, result);
    result = syssend(globalPIDRecv3, message);
    errorCheckingSend(3, result);
    result = syssend(globalPIDRecv4, message);
    errorCheckingSend(4, result);
    result = syssend(globalPIDRecv5, message);
    errorCheckingSend(0, result);
    result = syssend(globalPIDRecv5, message);
    errorCheckingSend(10, result);
    result = syssend(globalPIDRecv5, message);
    errorCheckingSend(20, result);
}

