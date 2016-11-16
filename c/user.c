/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

void producer( void ) {
/****************************/

    int         i;
    char        buff[100];
    syssleep(3000);
    for( i = 0; i < 20; i++ ) {
      
      sprintf(buff, "Producer %x and in hex %x %d\n", i+1, i, i+1);
      sysputs(buff);
      syssleep(1500);

    }
    for (i = 0; i < 15; i++) {
      sysputs("P");
      syssleep(1500);
    }
    sprintf(buff, "Producer finished\n");
    sysputs( buff );
    sysstop();
}

void consumer( void ) {
/****************************/

    int         i;
    char        buff[100];
    syssleep(3000);
    for( i = 0; i < 10; i++ ) {
      sprintf(buff, "Consumer %d\n", i);
      sysputs( buff );
      syssleep(1500);
      sysyield();
    }

    for (i = 0; i < 40; i++) {
      sysputs("C");
      syssleep(700);
    }

    sprintf(buff, "Consumer finished\n");
    sysputs( buff );
    sysstop();
}

void     root( void ) {
/****************************/

    char  buff[100];
    int proc_pid, con_pid;
    sysputs("Root has been called\n");
    syssleep(3000);
    sysyield();
    sysyield();
   
    proc_pid = syscreate( &producer, 4096 );
    con_pid = syscreate( &consumer, 4096 );
    sprintf(buff, "Proc pid = %d Con pid = %d\n", proc_pid, con_pid);
    sysputs( buff );

    sprintf(buff, "Root finished\n");
    sysputs( buff );
    sysstop();

    for( ;; ) {
     sysyield();
    }
}

