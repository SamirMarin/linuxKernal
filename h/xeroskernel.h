/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef	char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define	FALSE   0       /* Boolean constants             */
#define	TRUE    1
#define	EMPTY   (-1)    /* an illegal gpq                */
#define	NULL    0       /* Null pointer for linked lists */
#define	NULLCH '\0'     /* The null character            */


/* Universal return constants */

#define	OK            1         /* system call ok               */
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);
void           set_evec(unsigned int xnum, unsigned long handler);


// Global Constants 
#define PCBTABLESIZE 32
#define SIGNALMAX 31
#define DEVICETABLESIZE 2
#define FDTSIZE 4

//constants to track state that a process is in
#define STATE_STOPPED 0
#define STATE_READY 1
#define STATE_SLEEP 22
#define STATE_RUNNING 23
#define STATE_RECV 29
#define STATE_SEND 34 
#define STATE_WAITING 47
#define STATE_DEV_WAITING 92

//Time slice constant
#define TIMESLICE 100 // must change both these values. one is dependent on the other
#define TICKLENGTH 10 // assuming 1 tick is 10ms based on the TIMESLICE constant above

// init.c functions
extern struct pcb *pcbTable;
extern struct devsw *deviceTable;
// mem.c functions
extern void kmeminit(void);
extern void *kmalloc(int size);
extern void kfree(void *ptr);
// disp.c functions 
extern void dispatch(void);
extern struct pcb *readyQueueHead;// Head of pcb ready queue
extern struct pcb *readyQueueTail;// Tail of pcb ready queue
extern struct pcb *recvAnyQueueHead;//head of recv any queue
extern struct pcb *recvAnyQueueTail;//tail of recv any queue
extern struct pcb *stopQueueHead;// Head of pcb queue stopped
extern struct pcb *stopQueueTail;// Tail of pcb queue stopped
extern struct pcb *idleProcessHead; //points to idle process
extern struct pcb *idleProcessTail; //points to idle process
extern struct pcb *sleepQueueHead; //points to idle process
extern struct pcb *sleepQueueTail; //points to idle process
extern struct pcb* next(struct pcb **head, struct pcb **tail);
extern int ready(struct pcb *process, struct pcb **head, struct pcb **tail, int state);
extern void removeNthPCB(struct pcb *process);

//stores cpu context
struct CPU {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
    unsigned long iret_eip;
    unsigned long iret_cs;
    unsigned long eflags;
};
struct FD{
    int index;
    int majorNum;
    struct devsw *dvBlock;
    int status;
    char *name;
    struct FD *prev;
    struct FD *next;
}; 
//stores process information
struct pcb {
    int pid; // process ID
    int index; // index of pcb inside process table 
    int reuseCount; 
    int state; 
    unsigned long *args; // pointer to system call type and arguments on process stack
    unsigned long *memoryStart; //pointer to the allocated stack for the process
    unsigned long sp;//most current stack pointer for the process
    int rc;
    unsigned int tick;
    long cpuTime;
    unsigned long signalBitMask;
    void (*sigFunctions[SIGNALMAX+1])(void*); //array of function pointers for signal
    struct CPU *cpuState;// pointer to the cpu struct
    struct pcb *next;// pointer to next pcp in the queue
    struct pcb *prev;
    struct pcb **head; // pointer to the global variable that is a pointer to the head of the queue this pcb block belongs to
    struct pcb **tail; // pointer to the global variable that is a pointer to the tail of the queue this pcb block belongs to
    struct pcb *sendQHead;// pointer to the list of pcb's that want to send to this pcb 
    struct pcb *sendQTail;
    struct pcb *recvQHead;// pointer to the list of pcb's that want to recv from this pcb
    struct pcb *recvQTail;
    struct pcb *waitQHead;// pointer to the list of pcb's that are waiting for this process to die
    struct pcb *waitQTail;
    struct FD FDT[FDTSIZE]; //pointer to the FDT for this process always size four initiated with sysopen
};

// enum representing type of system calls available
enum SystemEvents {
    CREATE,
    YIELD,
    STOP,
    GETPID,
    PUTS,
    KILL,
    SEND,
    RECEIVE,
    TIMER_INT,
    SLEEP,
    CPU_TIMES,
    SIG_HANDLER,
    SIG_RETURN,
    WAIT,
    OPEN,
    CLOSE,
    WRITE,
    READ,
    IOCTL,
    KEYBOARD
};

struct processStatuses {
  int  pid[PCBTABLESIZE];      // The process ID
  int  status[PCBTABLESIZE];   // The process status
  long  cpuTime[PCBTABLESIZE]; // CPU time used in milliseconds
};

struct devsw{
    int dvnum;
    char *dvname;
    int (*dvopen)(struct devsw*, int);
    int (*dvclose)(struct devsw*);
    int (*dvread)(struct devsw*, struct pcb*, void*, int);
    int (*dvwrite)(struct devsw*, void*, int);
    int (*dvioctl)(struct devsw*, unsigned long, int);
    int *dvcsr;
    int *dvivec;
    int *dvovec;
    int (*dviint)(void);
    int (*dvoint)(void);
};
// di_calls.c functions
extern int di_open(struct pcb *process, int device_no);
extern int di_close(struct pcb *process, int fd);
extern int di_write(struct pcb *process, int fd, unsigned char *buff, int size);
extern int di_read(struct pcb *process, int fd, unsigned char *buff, int size);
extern int di_ioctl(struct pcb *process, int fd, unsigned long command, int val);

// ctsw.c functions
extern int contextswitch(struct pcb* process);
extern void contextinit(void);
// create.c 
extern int create(void (*func)(void), int stackSize);
extern int createIdle(void (*func)(void), int stackSize);
// syscall.c
extern int syscall(int call);
extern int syscall2(int call, ...);
extern void sysyield(void);
extern void sysstop(void);
extern unsigned int syscreate(void (*func)(void), int stack);
extern int sysgetpid( void );
extern void sysputs(char *str);
extern int syskill(int pid, int signalNumber);
extern int syssend(int dest_pid, unsigned long num);
extern int sysrecv(unsigned int *from_pid, unsigned long *num);
extern int syssleep( unsigned int milliseconds );
extern int sysgetcputimes(struct processStatuses *ps);
extern int syssighandler(int signal, void(*newHandler)(void*), void(**oldHandler)(void*));
extern int syssigreturn(void *old_sp);
extern int syswait(int pid);
extern int sysopen(int device_no);
extern int sysclose(int fd);
extern int syswrite(int fd, void *buff, int bufflen);
extern int sysread(int fd, void *buff, int bufflen);
extern int sysioctl(int fd, unsigned long command, ...);
// user.c 
extern void root(void);
// msg.c
extern int send(int dest_pid, unsigned long num, struct pcb * currentProcess);
extern int recv(unsigned int *from_pid, unsigned long *num, struct pcb * currentProcess);
// sleep.c
extern unsigned int sleep(unsigned int ms, struct pcb * process);
extern void tick(void);
// signal.c
extern int signal(int pid, int sig_no);
extern void sigtramp(void (*handler)(void*), void *cntx);
// kbd.c
int kbd_read_in(void);
int kb_open(const struct devsw* const dvBlock, int majorNum);
int kb_close(const struct devsw* const dvBlock);
int kb_ioctl(const struct devsw* const dvBlock, unsigned long command, int val);
int kb_read(const struct devsw * const dvBlock, struct pcb * const process, void *buff, int size);
int kb_write(const struct devsw * const dvBlock);






/* Anything you add must be between the #define and this comment */
#endif
