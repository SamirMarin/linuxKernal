/* mem.c : memory manager
*/

#include <xeroskernel.h>
#include <xeroslib.h>
#include <i386.h>

#define SANITYCHECK 314159265 // contant used for sanity check
extern long freemem; /* set in i386.c */
extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */

//structure used to store information about memory chunks
struct memHeader {
    unsigned long size;// size of the memory space allocated
    struct memHeader *prev; //previous memHeader on the freelist
    struct memHeader *next; //next memHeader on the freelist
    char *sanityCheck; // veryfy nothing is wrong with the memHeader returned by user
    unsigned char dataStart[0]; // start address of allocated memory
};


struct memHeader *freeListHead;
struct memHeader *freeListTail;




struct memHeader *removeFreeListMemberWithSize(int size);
int insert(struct memHeader *memSlot);
struct memHeader *removeFromList(struct memHeader *prevHead, struct memHeader *head);
struct memHeader *findNextMemHeaderWithAddress(long addr);
struct memHeader *findPreviousMemHeaderWithAddress(long addr);
void startTestMem(void);
void printCurrentList(char *descriptor);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  kmeminit
 *  Description:  initilizes the free memomry linked list
 * =====================================================================================
 */
void kmeminit(void) {
    struct memHeader *freeSpace1 = (struct memHeader*) freemem;
    freeSpace1->size = HOLESTART - freemem + sizeof(struct memHeader);
    freeSpace1->next = (struct memHeader*) HOLEEND;
    freeSpace1->sanityCheck = (char*) SANITYCHECK;

    struct memHeader *freeSpace2 = (struct memHeader*) HOLEEND;
    freeSpace2->size = 0x400000 - HOLEEND + sizeof(struct memHeader);
    freeSpace2->prev = (struct memHeader*) freemem;
    freeSpace2->sanityCheck = (char*) SANITYCHECK;

    int result = insert(freeSpace1);
    if (!result) {
        kprintf("\n\ninsert freespace1 failed\n file: mem.c\n function: kmeminit");
    }
    result = insert(freeSpace2);
    if (!result) {
        kprintf("\n\ninsert freespace2 failed\n file: mem.c\n function: kmeminit");
    }
    /*
       long addr1 = (long) (&(freeListHead->size));
       long addr2 = (long) (&(freeListTail->size));
       kprintf("\n\nfreespace1 allocated at %x\n, with size %u\n", addr1 , freeListHead->size);
       kprintf("\n\nfreespace2 allocated at %x\n, with size %u\n", addr2, freeListTail->size);
       */

    //startTestMem();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  kmalloc
 *  Description:  allocates memorys
 *       Return:  return pointer to allocated chunk if abs(size) is available.
 * =====================================================================================
 */
void* kmalloc(int size) {

    if (size < 1) {
        return NULL;
    }

    long finalSize = (size & 0xfffffff0) + 16;
    struct memHeader *foundNode = removeFreeListMemberWithSize(finalSize);
    //kprintf("final size: %u", finalSize);

    if (!foundNode) {
        kprintf("\n\ncould not find mem of: %d\n in function: kmalloc\n and file: mem.c", size);
        return NULL;
    } else {
        //kprintf("foundNode %u", foundNode);
        //kprintf("size memHeader %u", sizeof(struct memHeader));

        struct memHeader *newFreeSpace = (struct memHeader*) (((long)foundNode) + sizeof(struct memHeader) + finalSize);
        unsigned long newFreeSpaceLocation = (unsigned long) (newFreeSpace);
        //kprintf("addr of nfs %u", (long) newFreeSpace);

        // Don't insert a new free space header because we will be outside valid memory
        if ((newFreeSpaceLocation >= HOLESTART) || (newFreeSpaceLocation >= 0x400000)) {
            foundNode->size = finalSize;
            foundNode->sanityCheck = (char*) SANITYCHECK;
            return (void*) &(foundNode->dataStart);
        }

        newFreeSpace->size = foundNode->size - finalSize - sizeof(struct memHeader);
        newFreeSpace->sanityCheck = (char*) SANITYCHECK;
        int result = insert(newFreeSpace);

        if(!result){
            kprintf("\n\ninsert newFreeSpace failed\n file: mem.c\n function: kmalloc");
        }
        if(!newFreeSpace->size){
            kprintf("\n\ninserted memory with size 0\n file: mem.c\n function: kmalloc");
        }

        foundNode->size = finalSize;
        foundNode->sanityCheck = (char*) SANITYCHECK;
        return (void*) &(foundNode->dataStart);
    }
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  kfree
 *  Description:  adds memHeader to free memory link list given a pointer to the returned address
 *                if memHeader found to be invalid at given address goes into infinte loop.
 * =====================================================================================
 */
void kfree(void *ptr) {
    struct memHeader *returnedMemHeader = (struct memHeader *) (((long) ptr) - sizeof(struct memHeader));
    if(((long)returnedMemHeader->sanityCheck != SANITYCHECK)){
        kprintf("\n\n not a match or memhead overwritten file: mem.c function: kfree");
        for(;;);
    }
    //not sure if this is needed
    returnedMemHeader->sanityCheck = NULL;
    long returnedMemHeaderStartAddr = (long) returnedMemHeader;
    long returnedMemHeaderEndAddr = returnedMemHeaderStartAddr + sizeof(struct memHeader) + returnedMemHeader->size;

    struct memHeader *previousMemHeader = findPreviousMemHeaderWithAddress(returnedMemHeaderStartAddr);
    struct memHeader *nextMemHeader = findNextMemHeaderWithAddress(returnedMemHeaderEndAddr);
    // found all three or just the previous
    if (previousMemHeader) {
        // we update previous memheader
        previousMemHeader->size += (returnedMemHeader->size + sizeof(struct memHeader));
        previousMemHeader->sanityCheck = NULL;
        if (nextMemHeader) {
            previousMemHeader->size += (nextMemHeader->size + sizeof(struct memHeader));
            // insert into list
        }
        insert(previousMemHeader);
        return;
    }
    // found the current and the next;
    if (nextMemHeader) {
        // update current
        returnedMemHeader->size += (nextMemHeader->size + sizeof(struct memHeader));
        // insert into list
        insert(returnedMemHeader);
        return;
    }
    // else just insert the returned memheader;
    insert(returnedMemHeader);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  insert
 *  Description:  performs actual insert of memHeader into freelist
 * =====================================================================================
 */
int insert(struct memHeader *memSlot) {
    if(!freeListTail && !freeListHead){
        freeListHead = memSlot;
        freeListTail = memSlot;
        return 1;
    }
    if(freeListHead && freeListTail){
        struct memHeader *current = freeListHead;
        struct memHeader *prevCurrent = NULL;
        long addressMemSlot = (long) memSlot;
        while(current){
            if(addressMemSlot < (long) current){
                if(!prevCurrent){
                    memSlot->next = current;
                    current->prev = memSlot;
                    freeListHead = memSlot;
                    return 1;
                }
                else{
                    prevCurrent->next = memSlot;
                    current->prev = memSlot;
                    memSlot->next = current;
                    memSlot->prev = prevCurrent;
                    return 1;
                }
            }
            prevCurrent = current;
            current = current->next;
        }
        memSlot->prev = freeListTail;
        freeListTail->next = memSlot;
        freeListTail = freeListTail->next;
        return 1;
    }
    kprintf("\n\n one of freeListHead or freeListTail is NULL\n file: mem.c\n function: insert");
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  removeFreeeListMemberWithSize
 *  Description:  removes the first item in the freelist >= given size
 *       Return:  returns the memHeader removed from freelist
 * =====================================================================================
 */
struct memHeader *removeFreeListMemberWithSize(int size) {
    struct memHeader *head = freeListHead;
    struct memHeader *prevHead = NULL;
    while(head){
        if(head->size >= size){
            return removeFromList(prevHead, head);
        }
        prevHead = head;
        head = head->next;
    }
    //kprintf("Error: Not enough free memory found.\n");
    return NULL;
}

struct memHeader *removeFromList(struct memHeader *prevHead, struct memHeader *head) {
    //case: find first item on list;
    if(!prevHead){
        //case: head and tail are equal
        if (freeListHead == freeListTail){
            freeListHead = NULL;
            freeListTail = NULL;
        }
        //case: head and tail are not equal
        else{
            freeListHead = head->next;
            freeListHead->prev = NULL;
            head->next = NULL;
        }
    }
    //case: find nth item in list
    else{
        if(head == freeListTail){
            freeListTail = freeListTail->prev;
            head->prev = NULL;
        }
        else{
            struct memHeader *oneAhead = head->next;
            if (oneAhead) {
                oneAhead->prev = prevHead;
            }
            prevHead->next = oneAhead;
            head->next = NULL;
            head->prev = NULL;
        }
    }
    return head;

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  findNextMemHeaderWithAddress
 *  Description:  finds a memHeader with given address that comes next
 *       Return:  returns found memHeader if found else Null
 * =====================================================================================
 */
struct memHeader *findNextMemHeaderWithAddress(long addr) {
    struct memHeader *head = freeListHead;
    struct memHeader *prevHead = NULL;
    while(head){
        long headAddress = (long) head;
        if (headAddress == addr) {
            return removeFromList(prevHead, head);
        }
        prevHead = head;
        head = head->next;
    }
    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  findPreviousMemHeaderWithAddress
 *  Description:  finds a memHeader with given address that comes previous
 *       Return:  returns found memHeader if found else Null
 * =====================================================================================
 */
struct memHeader *findPreviousMemHeaderWithAddress(long addr) {
    struct memHeader *head = freeListHead;
    struct memHeader *prevHead = NULL;
    while(head){
        long headAddress =  (((long)head) + sizeof(struct memHeader) + head->size);
        if (headAddress == addr) {
            return removeFromList(prevHead, head);
        }
        prevHead = head;
        head = head->next;
    }
    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  startTestMem
 *  Description:  starting method for testing memory allocation.
 * =====================================================================================
 */
void startTestMem(void){
    printCurrentList("start");
    //test a simple allocation
    int* firstAllocation = (int*) kmalloc(1000);
    //allocation should have moved over 1024 size 1008
    printCurrentList("1000");
    //allocate second item
    int* secondAllocation = (int*) kmalloc(2000);
    //allocation should have moved over 2032 size 2016
    printCurrentList("2000");
    int* thirdAllocation = (int*) kmalloc(5000);
    //allocation should have moved over 5024 size 5008
    printCurrentList("5000");
    //kfree the second item
    kfree(thirdAllocation);
    printCurrentList("free3");
    kfree(firstAllocation);
    printCurrentList("free1");
    kfree(secondAllocation);
    printCurrentList("free2");
}

void printCurrentList(char *descriptor){
    int i = 0;
    struct memHeader *current = freeListHead; 
    kprintf("list %s\n", descriptor);
    while(current){
        long address = (long) current;
        kprintf("(mem: %u/size: %u)-->", address, current->size);
        i = i + 1;
        current = current->next;
    }
    kprintf("\n");
}
