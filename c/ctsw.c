/* ctsw.c : context switcher
*/

#include <xeroskernel.h>

/* Your code goes here - You will need to write some assembly code. You must
   use the gnu conventions for specifying the instructions. (i.e this is the
   format used in class and on the slides.) You are not allowed to change the
   compiler/assembler options or issue directives to permit usage of Intel's
   assembly language conventions.
   */
void _ISREntryPoint(void);
void _TimerEntryPoint(void);
static void *k_stack;
static unsigned long ESP; 
static int rc, interrupt;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  contextswitch
 *  Description:  switches between kernal and process (and vice versa)
 *       Return:  returns an int representing the type of system call, -1 if 
 *                given process is null
 * =====================================================================================
 */
int contextswitch(struct pcb* process){
    if(!process){
        return -1;
    }

    ESP = process->sp;
    unsigned long *eax_register = (unsigned long*) (ESP + 28);
    *eax_register = process->rc;

    __asm__ __volatile__("pushf\n\t"
            "pusha\n\t"
            "movl %%esp, k_stack\n\t"
            "movl ESP, %%esp\n\t"
            "popa\n\t"
            "iret\n"
            "_TimerEntryPoint:\n\t"
            "cli\n\t"
            "pusha\n\t"
            "movl $1, %%ecx\n\t"
            "jmp _CommonEntryPoint\n"
            "_ISREntryPoint:\n\t"
            "cli\n\t"
            "pusha\n\t"
            "movl $0, %%ecx\n"
            "_CommonEntryPoint:\n\t"
            "movl %%esp, ESP\n\t"
            "movl k_stack, %%esp\n\t"
            "movl %%eax, rc\n\t"
            "movl %%ecx, interrupt\n\t"
            "popa\n\t"
            "popf"
            :
            :
            :"%eax", "%ecx"
            );

    process->args = (unsigned long*) (ESP + 44);

    if (interrupt) {
        unsigned long *eax_register = (unsigned long*) (ESP + 28);
        process->rc = *eax_register;
        rc = TIMER_INT;
    } else {
        rc = *(process->args);
    }
    process->sp = ESP;

    /*
       kprintf("\n\ncreate value %d", *(process->args));
       kprintf("\n\naddress fucntion %d", *(process->args + 1));
       kprintf("\n\nstack size %d", *(process->args + 2));
       */

    return rc;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  contextinit
 *  Description:  sets entry points to inturrupt table
 * =====================================================================================
 */
void contextinit(void){
    set_evec(67, (unsigned long) _ISREntryPoint);
    set_evec(32, (unsigned long) _TimerEntryPoint);
}
