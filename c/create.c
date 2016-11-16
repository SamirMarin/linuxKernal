/* create.c : create a process
 */

#include <xeroskernel.h>
#include <xeroslib.h>

pcb     proctab[MAX_PROC];

/* make sure interrupts are armed later on in the kernel development  */
#define STARTING_EFLAGS         0x00003000
#define ARM_INTERRUPTS          0x00000200


// PIDs can't start at 0 nor can they be negative
static int      nextpid = 1;



int create( funcptr fp, size_t stackSize ) {
/***********************************************/

    context_frame       *cf;
    pcb                 *p = NULL;
    int                 i;


    /* PID has wrapped and we can't have -ve numbers 
     * this means that the next PID we handout can't be
     * in use. To find such a number we have to propose a 
     * new PID and then scan to see if it is in the table. If it 
     * is then we have to try again. We know we will succeed 
     * because the process table size is smaller than PID space.
     * However, this code does not do that and just returns an 
     * error.
     */


    if (nextpid < 0) 
      return CREATE_FAILURE;

    // If the stack is too small make it larger
    if( stackSize < PROC_STACK ) {
        stackSize = PROC_STACK;
    }

    for( i = 0; i < MAX_PROC; i++ ) {
        if( proctab[i].state == STATE_STOPPED ) {
            p = &proctab[i];
            break;
        }
    }
    
    //    Some stuff to help wih debugging
    //    char buf[100];
    //    sprintf(buf, "Slot %d empty\n", i);
    //    kprintf(buf);
    //    kprintf("Slot %d empty\n", i);
    
    if( !p ) {
        return CREATE_FAILURE;
    }


    cf = kmalloc( stackSize );
    if( !cf ) {
        return CREATE_FAILURE;
    }

    cf = (context_frame *)((unsigned char *)cf + stackSize - 4);
    cf--;

    memset(cf, 0xA5, sizeof( context_frame ));

    cf->iret_cs = getCS();
    cf->iret_eip = (unsigned int)fp;
    cf->eflags = STARTING_EFLAGS | ARM_INTERRUPTS;

    cf->esp = (int)(cf + 1);
    cf->ebp = cf->esp;
    p->esp = (unsigned long*)cf;
    p->state = STATE_READY;
    p->pid = nextpid++;

    ready( p );
    return p->pid;
}
