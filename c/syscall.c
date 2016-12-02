/* syscall.c : syscalls
*/

#include <xeroskernel.h>
#include <stdarg.h>
#include <stdio.h>

/* Your code goes here */


int syscall(int call);
int syscall2(int call, ...);
int syscall3(int call, ... );
int syscall4(int call, ... );
void sysyield( void );
unsigned int syscreate( void (*func)(void), int stack);
void sysstop( void );
int sysgetpid( void );
void sysputs(char *str);
int syskill(int pid, int signalNumber);
int syssend(int dest_pid, unsigned long num);
int sysrecv(unsigned int *from_pid, unsigned long *num);
int syssleep( unsigned int milliseconds );
int syssighandler(int signal, void(*newHandler)(void*), void(**oldHandler)(void*));
int syssigreturn(void *old_sp);

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

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  syscall4
 *  Description:  pushes four arguments on to stack
 * =====================================================================================
 */
int syscall4(int call, ... ){
    int result = 0;
    __asm__ __volatile__(
            "pushl 20(%%ebp)\n\t"
            "pushl 16(%%ebp)\n\t"
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

int syskill(int pid, int signalNumber) {
    int res = syscall3(KILL, pid, signalNumber);
    if (res == -1) {
        return -712;
    } else if (res == -2) {
        return -651;
    } else {
        return 0;
    }
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

int sysgetcputimes(struct processStatuses *ps) {
    if (!ps) {
        return -2;
    }
    return syscall2(CPU_TIMES, ps);
}

int syssighandler(int signal, void(*newHandler)(void *), void(**oldHandler)(void*)) {
        return syscall4(SIG_HANDLER, signal, newHandler, oldHandler);

}

int syssigreturn(void *old_sp) {
    return syscall2(SIG_RETURN,  old_sp);
}

int syswait(int pid) {
    return syscall2(WAIT, pid);
}

int sysopen(int device_no){
    return syscall2(OPEN, device_no);
}

int sysclose(int fd){
    return syscall2(CLOSE, fd);
}

int syswrite(int fd, void *buff, int bufflen){
    return syscall4(WRITE, fd, buff, bufflen);
}

int sysread(int fd, void *buff, int bufflen){
    return syscall4(READ, fd, buff, bufflen);
}

int sysioctl(int fd, unsigned long command, ...){
        int val = 0;
        va_list list;
        va_start(list, 1);
        val += va_arg(list, int);
        va_end(list);
        return syscall4(IOCTL, fd, command, val);
}
