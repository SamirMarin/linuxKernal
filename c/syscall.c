/* syscall.c : syscalls
*/

#include <xeroskernel.h>
#include <stdarg.h>

/* Your code goes here */


int syscall(int call);
int syscall2(int call, ...);
void sysyield( void );
unsigned int syscreate( void (*func)(void), int stack);
void sysstop( void );
int sysgetpid( void );
void sysputs(char *str);
int syskill(int pid);
int syssend(int dest_pid, unsigned long num);
int sysrecv(unsigned int *from_pid, unsigned long *num);
int syssleep( unsigned int milliseconds );

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  syscall
 *  Description:  pushes one arguments on to stack 
 * =====================================================================================
 */
int syscall(int call){
    int result = 0;
    __asm__ __volatile__("pushl 8(%%ebp)\n\t"
            "int $67\n\t"
            "movl %%eax, %0\n\t"
            "popl %%eax"
            :"=r"(result)
            : 
            :"%eax"
            ); 
    return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  syscall2
 *  Description:  pushes two arguments on to stack 
 * =====================================================================================
 */
int syscall2(int call, ... ){
    int result = 0;
    __asm__ __volatile__("pushl 12(%%ebp)\n\t"
            "pushl 8(%%ebp)\n\t"
            "int $67\n\t"
            "movl %%eax, %0\n\t"
            "popl %%eax\n\t"
            "popl %%eax"
            : "=r"(result)
            : 
            :"%eax"
            );
    return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  syscall3
 *  Description:  pushes three arguments on to stack 
 * =====================================================================================
 */
int syscall3(int call, ... ){
    int result = 0;
    __asm__ __volatile__("pushl 16(%%ebp)\n\t"
            "pushl 12(%%ebp)\n\t"
            "pushl 8(%%ebp)\n\t"
            "int $67\n\t"
            "movl %%eax, %0\n\t"
            "popl %%eax\n\t"
            "popl %%eax\n\t"
            "popl %%eax"
            : "=r"(result)
            : 
            :"%eax"
            );
    return result;
}

unsigned int syscreate( void (*func)(void), int stack){
    if (!func) {
        return 0;
    }
    return syscall3(CREATE, func, stack);
}

void sysyield( void ){
    syscall(YIELD);
}

void sysstop( void ){
    syscall(STOP);
}

int sysgetpid( void ) {
    return syscall(GETPID);
}

void sysputs(char *str) {
    if (!str) {
        return;
    }
    syscall2(PUTS, str);
}

int syskill(int pid) {
    return syscall2(KILL, pid);
}

int syssend(int dest_pid, unsigned long num) {
    if (dest_pid < 1) {
        return -1;
    }
    return syscall3(SEND, dest_pid, num);
}

int sysrecv(unsigned int *from_pid, unsigned long *num) {
    if (!from_pid || !num) {
        return -3;
    }
    return syscall3(RECEIVE, from_pid, num);
}

int syssleep( unsigned int milliseconds ){
    return syscall2(SLEEP, milliseconds);

}

int sysgetcputimes(processStatuses *ps) {
    if (!ps) {
        return -2;
    }
    return syscall2(SYS_CPUTIMES, ps);
}
