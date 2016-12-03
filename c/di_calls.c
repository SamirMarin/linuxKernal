
/* di_calls.c : device independent calls 
*/

#include <xeroskernel.h>


int di_open(struct pcb *process, int device_no);
int di_close(struct pcb *process, int fd);
int di_write(struct pcb *process, int fd, unsigned char *buff, int size);
int di_read(struct pcb *process, int fd, unsigned char *buff, int size);
int di_ioctl(struct pcb *process, int fd, unsigned long command, int val);
void initFDT( struct pcb *process );
int addToQueue(struct FD *fd, struct FD **head, struct FD **tail);
struct FD* nextFd(struct FD *head);
int validDescr(struct pcb *process, int fd);

int di_open(struct pcb *process, int device_no){
    if(device_no >= DEVICETABLESIZE){
        return -1;
    }
    struct FD *fdNew = nextFd(process->FDT);
    struct devsw *devopenptr;
    if(!fdNew){
        return -1;
    }
    fdNew->majorNum = device_no;
    fdNew->status = 1; // 1 marks fd entry as currently open by device
    devopenptr = &deviceTable[device_no];
    fdNew->dvBlock = devopenptr;
    int result = (devopenptr->dvopen)(devopenptr, device_no);
    if(result){
        return result;
    }
    return fdNew->index;
}

int di_close(struct pcb *process, int fd){
    struct devsw *devcloseptr;
    int result = validDescr(process, fd);
    if(!result){
        return -1;
    }
    devcloseptr = process->FDT[fd].dvBlock;
    process->FDT[fd].status = 0;
    process->FDT[fd].majorNum = -1;
    int res = (devcloseptr->dvclose)(devcloseptr);
    process->FDT[fd].dvBlock = NULL;
    return res;
}

int di_write(struct pcb *process, int fd, unsigned char *buff, int size){
    struct devsw *devwriteptr;
    int result = validDescr(process, fd);
    if(!result || !buff || !size){
        return -1;
    }
    devwriteptr = process->FDT[fd].dvBlock;
    return (devwriteptr->dvwrite)(devwriteptr);
}

int di_read(struct pcb *process, int fd, unsigned char *buff, int size){
    struct devsw *devreadptr;
    int result = validDescr(process, fd);
    if(!result || !buff || !size){
        return -1;
    }
    devreadptr = process->FDT[fd].dvBlock;
    return (devreadptr->dvread)(devreadptr, process, buff, size);
}
int di_ioctl(struct pcb *process, int fd, unsigned long command, int val){
    struct devsw *devioctlptr;
    int result = validDescr(process, fd);
    if(!result){
        return -1;
    }
    devioctlptr = process->FDT[fd].dvBlock;
    return (devioctlptr->dvioctl)(devioctlptr, command, val);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  nextFd
 *  Description:  removes a FD struct from given list identified by the given head.
 *       Return:  return FD struct, null if non availble in given list
 * =====================================================================================
 */
struct FD* nextFd(struct FD *head){
    if(!head){
        return NULL;
    }
    int i;
    for(i = 0; i < FDTSIZE; i++){
        if(head[i].majorNum != -1){
            return NULL;
        }
    }
    return head; 
}
int validDescr(struct pcb *process, int fd){
    struct FD *fdEntry = &(process->FDT[fd]);
    if(fd < 0 || fd > 3 || !fdEntry->status){
        return 0; // invalid 
    }
    return 1;
}



