
/* di_calls.c : device independent calls 
*/

#include <xeroskernel.h>
#include <stdarg.h>

#define FDTSIZE 4

void initFDT( struct pcb *process );
int addToQueue(struct FD *fd, struct FD **head, struct FD **tail);
struct FD* nextFd(struct FD **head, struct FD **tail);
int validDescr(struct pcb *process, int fd);

int di_open(struct pcb *process, int device_no){
    if(!process->FDT){
        initFDT(process);
    }
    struct FD *fdNew = nextFd(&process->FDTFreeHead, &process->FDTFreeTail);
    if(!fdNew){
        //all file descriptor in FDT currenlty in use for the process;
        return -1;
    }
    fdNew->majorNum = device_no;
    fdNew->status = 1; // 1 marks fd entry as currently open by device
    fdNew->dvBlock = &deviceTable[device_no];
    //TODO: need to call the specific device open !!!
    return fdNew->index;
}

int di_close(struct pcb *process, int fd){
    struct devsw *devcloseptr;
    int result = validDescr(process, fd);
    if(!result){
        //system error-- Not sure if we want to return differently depending on the error
        //out of range or not an opend device?? 
        return -1;
    }
    devcloseptr = process->FDT[fd].dvBlock;
    //here we need to call the our implimentation in devsw block
    //TODO: make appropriate call with appropriate parameters
    return (devcloseptr->dvclose)(devcloseptr);

}

int di_write(struct pcb *process, int fd, unsigned char *buff, int size){
    struct devsw *devwriteptr;
    int result = validDescr(process, fd);
    if(!result){
        //system error-- Not sure if we want to return differently depending on the error
        //out of range or not an opend device?? 
        return -1;
    }
    devwriteptr = process->FDT[fd].dvBlock;
    //TODO: fix this return according to how we implement the write
    return (devwriteptr->dvwrite)(devwriteptr, buff, size);

}

int di_read(struct pcb *process, int fd, unsigned char *buff, int size){
    struct devsw *devreadptr;
    int result = validDescr(process, fd);
    if(!result){
        //system error-- Not sure if we want to return differently depending on the error
        //out of range or not an opend device?? 
        return -1;
    }
    devreadptr = process->FDT[fd].dvBlock;
    //TODO: fix this return according to how we implement the write
    return (devreadptr->dvwrite)(devreadptr, buff, size);

}

void initFDT( struct pcb *process){
    struct FD *fdt = (struct FD*) kmalloc(FDTSIZE*sizeof(struct FD));
    process->FDT = fdt;
    int i;
    for(i = 0; i < FDTSIZE; i++){
        process->FDT[i].index = i;
        addToQueue(process->FDT + i, &process->FDTFreeHead, &process->FDTFreeTail);
    }
    
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  addToQueue
 *  Description:  adds a file descriptor to given list identified by head and tail.
 *       Return:  1 if sucessful 0 if unsucessful
 * =====================================================================================
 */
int addToQueue(struct FD *fd, struct FD **head, struct FD **tail){
    if(!fd){
        return -1;
    }
    if(!*head && !*tail){
        *head = fd;
        *tail = fd;
        fd->prev = NULL;
        fd->next = NULL;
        return 1;
    }
    if(*head && *tail){
        (*tail)->next = fd;
        fd->prev = *tail;
        *tail = fd;
        (*tail)->next = NULL;
        return 1;
    }
    kprintf("\n\n one of queueHead or queueTail is NULL\n file: disp.c\n function: ready");
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  nextFd
 *  Description:  removes a FD struct from given list identified by the given head.
 *       Return:  return FD struct, null if non availble in given list
 * =====================================================================================
 */
struct FD* nextFd(struct FD **head, struct FD **tail){
    if(!*head){
        return NULL;
    }
    //if they are the same only one thing on list
    if (*head == *tail) {
        *tail = NULL;
    }
    struct FD *nextFd = *head;
    *head = (*head)->next;
    nextFd->next = NULL;
    nextFd->prev = NULL;
    (*head)->prev = NULL;
    return nextFd;
}
int validDescr(struct pcb *process, int fd){
    if(fd < 0 || fd > 3){
        return -1; // invalid fd range
    }
    struct FD *fdEntry = &(process->FDT[fd]);
    if(!fdEntry->status){
        return -2; // not an open device
    }
    return 1;
}
