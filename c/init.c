/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>

extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/

struct pcb *pcbTable;

void initProcessTable( void );
static void idleproc( void );

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc( void )				/* The beginning */
{

    char str[1024];
    int a = sizeof(str);
    int b = -69;
    int i; 

    kprintf( "\n\nCPSC 415, 2016W1 \n32 Bit Xeros 0.01 \nLocated at: %x to %x\n", 
            &entry, &end); 

    kprintf("Some sample output to illustrate different types of printing\n\n");

    /* A busy wait to pause things on the screen, Change the value used 
       in the termination condition to control the pause
       */

    for (i = 0; i < 3000000; i++);

    /* Build a string to print) */
    sprintf(str, 
            "This is the number -69 when printed signed %d unsigned %u hex %x and a string %s.\n      Sample printing of 1024 in signed %d, unsigned %u and hex %x.",
            b, b, b, "Hello", a, a, a);

    /* Print the string */

    kprintf("\n\nThe %dstring is: \"%s\"\n\nThe formula is %d + %d = %d.\n\n\n", 
            a, str, a, b, a + b);

    for (i = 0; i < 4000000; i++);
    /* or just on its own */
    kprintf(str);

    /* Add your code below this line and before next comment */
    kprintf("\n");
    kmeminit();
    contextinit();
    initPIT(TIMESLICE);
    pcbTable = (struct pcb*) kmalloc(PCBTABLESIZE*sizeof(struct pcb));
    if (!pcbTable) {
        kprintf("\n\nCould not allocate memory for pcbtable. File: init.c. Function: initproc()");
        for(;;);
    }
    initProcessTable();

    create(&root, 8000);
    createIdle(&idleproc, 8000);
    //testing function for stopping process.
    //int restest = create(&testStop, 7989);
    //kprintf("Root PID is %d", res);
    dispatch();


    for (i = 0; i < 2000000; i++);
    /* Add all of your code before this comment and after the previous comment */
    /* This code should never be reached after you are done */
    kprintf("\n\nWhen the kernel is working properly ");
    kprintf("this line should never be printed!\n");
    for(;;) ; /* loop forever */
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  initProcessTable
 *  Description:  initiate the process table putting it on the stoppedQueue
 * =====================================================================================
 */
void initProcessTable( void ){
    // Allocate a process table with 32 pcbs and place them all in the stopped queue
    struct pcb *pcbTableHead = pcbTable; 
    int i;
    for (i = 0; i < PCBTABLESIZE; i++) {
        pcbTableHead[i].pid = -1;
        pcbTableHead[i].index = i+1;
        pcbTableHead[i].reuseCount = -1;
        pcbTableHead[i].head = &stopQueueHead;
        pcbTableHead[i].tail = &stopQueueTail;
        ready(pcbTableHead+i, &stopQueueHead, &stopQueueTail);
    }
    //struct pcb *pcbTableHeadTest = pcbTable;
    /* test to make sure that stopped queue is set up with 32 pcbs
       i = 0;
       while(i < PCBTABLESIZE){
       kprintf("file: init.c, functin: initproc state: %d\n",  (pcbTableHeadTest+i)->state);
       i++;
       }
       */

}

static void idleproc( void ){
    for(;;){
        sysyield();
    }
}
