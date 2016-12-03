/* user.c : User processes
*/

#include <xeroskernel.h>
#include <xeroslib.h>

void busy( void ) {
  int myPid;
  char buff[100];
  int i;
  int count = 0;

  myPid = sysgetpid();
  
  sprintf(buff, "My pid is %d \n", myPid);
  sysputs(buff);
  if (myPid == 2 ) {
      syskill(3, 31);
      syskill(3, 28);
  }
  sysyield();
  if (myPid == 4){
      syskill(5, 31);
      syskill(5, 28);
  }
  sysyield();
}

void printFunctionTosignal( void ){
    kprintf("\n you im a signal");
}
void testsignalpriority( void ){
    void (*oldhandlerptr) (void*);
    int result =  syssighandler(28, &sysstop, &oldhandlerptr);
    if(!result){
        kprintf("\n error in syshandler");
    }
    int result2 = syssighandler(31, &printFunctionTosignal, &oldhandlerptr);
    if(!result2){
        kprintf("\n error in syshandler sysgetpid");
    }
    int proc_pid = syscreate(&busy, 1024);
    int con_pid = syscreate(&busy, 1024);
    int result3 = syssighandler(31, &sysstop, &oldhandlerptr);
    if(!result3){
        kprintf("\n error in syshandler sysgetpid");
    }
    int result4 = syssighandler(28, oldhandlerptr, &oldhandlerptr);
    if(!result4){
        kprintf("\n error in syshandler sysgetpid");
    }
    int proc_pid2 = syscreate(&busy, 1024);
    int con_pid2= syscreate(&busy, 1024);

}
void     root( void ) {
/****************************/

    char  buff[100];
    int pids[5];
    int proc_pid, con_pid;
    int i;

    int rootPid = sysgetpid();
    sysputs("Root has been called\n");
    sprintf(buff, "Root pid is %d\n", rootPid);
    sysputs(buff);

    testsignalpriority();
    kprintf("\n done with root");

    // Test for ready queue removal. 
  /* 
    proc_pid = syscreate(&busy, 1024);
    con_pid = syscreate(&busy, 1024);
    sysyield();
    syskill(proc_pid, 31);
    sysyield();
    syskill(con_pid, 31);

    
    for(i = 0; i < 5; i++) {
      pids[i] = syscreate(&busy, 1024);
    }

    sysyield();
    
    syskill(pids[3], 31);
    sysyield();
    syskill(pids[2], 31);
    syskill(pids[4], 31);
    sysyield();
    syskill(pids[0], 31);
    sysyield();
    syskill(pids[1], 31);
    sysyield();

    syssleep(8000);;



    kprintf("***********Sleeping no kills *****\n");
    // Now test for sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    sysyield();
    syssleep(8000);;



    kprintf("***********Sleeping kill 2000 *****\n");
    // Now test for removing middle sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    syssleep(110);
    syskill(pids[1], 31);
    syssleep(8000);;

    kprintf("***********Sleeping kill last 3000 *****\n");
    // Now test for removing last sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    sysyield();
    syskill(pids[2], 31);
    syssleep(8000);;

    kprintf("***********Sleeping kill first process 1000*****\n");
    // Now test for first sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    syssleep(100);
    syskill(pids[0], 31);
    syssleep(8000);;

    // Now test for 1 process


    kprintf("***********One sleeping process, killed***\n");
    pids[0] = syscreate(&sleep2, 1024);

    sysyield();
    syskill(pids[0], 31);
    syssleep(8000);;

    kprintf("***********One sleeping process, not killed***\n");
    pids[0] = syscreate(&sleep2, 1024);

    sysyield();
    syssleep(8000);;



    kprintf("***********Three sleeping processes***\n");    // 
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);


    // Producer and consumer started too
    proc_pid = syscreate( &producer, 4096 );
    con_pid = syscreate( &consumer, 4096 );
    sprintf(buff, "Proc pid = %d Con pid = %d\n", proc_pid, con_pid);
    sysputs( buff );


    struct processStatuses psTab;
    int procs;
    



    syssleep(500);
    procs = sysgetcputimes(&psTab);

    for(int j = 0; j <= procs; j++) {
      sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
	      psTab.cpuTime[j]);
      kprintf(buff);
    }


    syssleep(10000);
    procs = sysgetcputimes(&psTab);

    for(int j = 0; j <= procs; j++) {
      sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
	      psTab.cpuTime[j]);
      kprintf(buff);
    }

    sprintf(buff, "Root finished\n");
    sysputs( buff );
    sysstop();
    
    for( ;; ) {
     sysyield();
    }*/
    
}

