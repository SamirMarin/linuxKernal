
/* di_calls.c : device independent calls 
*/

#include <xeroskernel.h>


int di_open(struct pcb *process, int device_no);
int di_close(struct pcb *process, int fd);
int di_write(struct pcb *process, int fd, unsigned char *buff, int size);
int di_read(struct pcb *process, int fd, unsigned char *buff, int size);
void initFDT( struct pcb *process );
int addToQueue(struct FD *fd, struct FD **head, struct FD **tail);
struct FD* nextFd(struct FD *head);
int validDescr(struct pcb *process, int fd);

int di_open(struct pcb *process, int device_no){
    struct FD *fdNew = nextFd(process->FDT);
    if(!fdNew){
        //one file descriptor in FDT currenlty in use for the process;
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
    process->FDT[fd].status = 0;
    process->FDT[fd].majorNum = -1;
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
    int res = (devreadptr->dvread)(devreadptr, buff, size);
    if (!res) {
        // read error handling specific stuff
    }
    return res;

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



