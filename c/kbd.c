#include <xeroskernel.h>
#include <i386.h>
#include <kbd.h>

static char kbuf[MAX_KBUF_SIZE];
static int kBytesRead = 0;

static char TOGGLE_ECHO = 1;
static char KB_IN_USE = 0;
static int EOFINDICATOR = 0x4;

// Stores exactly one request for the keyboard
static struct dataRequest kbDataRequest;

static int state; /* the modifer key state of the keyboard */

unsigned int kbtoa( unsigned char code );
int done(int retCode);
int kb_open(const struct devsw* const dvBlock, int majorNum);
int kb_close(const struct devsw* const dvBlock);
int kb_ioctl(const struct devsw* const dvBlock, unsigned long command, int val);
int kb_write(const struct devsw * const dvBlock);
int kb_read(const struct devsw * const dvBlock, struct pcb * const process, void *buff, int size);
int copyCharactersToBuffer(char *outBuf, int outBufSize, int outBufBytesRead, char * inBuf, int inBufSize, int *inBufBytesRead);
static int extchar(unsigned char code);

int kb_open(const struct devsw* const dvBlock, int majorNum) {
    if (KB_IN_USE) {
        //return failure
        return -1;
    }

    if (!majorNum) {
        // Make sure echo is turned off
        TOGGLE_ECHO = 0;
    }
    if (majorNum) {
        TOGGLE_ECHO = 1;
    }
    // Getting a compiler warning for this one
    KB_IN_USE = 1;
    //enable_irq(1,0);
    return 0;
}

int kb_write(const struct devsw * const dvBlock) {
    //we can just always return -1 here since we don't write
    return -1;
}

int kb_close(const struct devsw* const dvBlock) {
    if (!KB_IN_USE) {
        //return failure
        return -1;
    }
    if (!dvBlock->dvnum) {
        // closing steps for device one
    }
    if (dvBlock->dvnum == 1) {
        //clsing steps for device two

    }
    kBytesRead = 0;
    KB_IN_USE = 0;
    return 0;
}

int kb_ioctl(const struct devsw* const dvBlock, unsigned long command, int val) {
    if(command == 53){
        EOFINDICATOR = val;
        return 0;
    }
    else if(command == 56){
        TOGGLE_ECHO = 1;
        return 0;
    }
    else if(command == 55){
        TOGGLE_ECHO = 0;
        return 0;
    }
    else{
        return -1; //error didn't get correct command
    }
}


int kb_read(const struct devsw * const dvBlock, struct pcb * p, void *buff, int size) {
    if (dvBlock->dvnum) {
        // device 1 specific stuff


    }
    if (!dvBlock->dvnum) {
        // device 0 specific stuff

    }
    int bytesRead = 0;
    if (kBytesRead > 0) {
        bytesRead = copyCharactersToBuffer(buff, size, 0, &kbuf[0], MAX_KBUF_SIZE, &kBytesRead);
    }
    if (bytesRead == size) {
        // we're done with this sysread call;
        return 0;
    }
    kbDataRequest.status = 1;
    kbDataRequest.buff = buff;
    kbDataRequest.size = size;
    kbDataRequest.bytesRead = bytesRead;
    kbDataRequest.done = &done;
    kbDataRequest.blockedProc = p;
    p->state = STATE_DEV_WAITING;
    return 0;
}


int done(int retCode) {

    struct pcb * p =  kbDataRequest.blockedProc;
    kbDataRequest.status = 0;
    kbDataRequest.blockedProc = NULL;
    kbDataRequest.size = 0;
    kbDataRequest.bytesRead = 0;
    p->rc = retCode;
    ready(p, &readyQueueHead, &readyQueueTail, STATE_READY);
    return 0;
}


//===================================
//          LOWER HALF
//===================================

int kbd_read_in() {
    unsigned char ctrlByte = inb(CTRL_PORT);
    unsigned char scanCode = inb(READ_PORT);
    unsigned long parsedCharacter = kbtoa(scanCode);
    if (!(ctrlByte & 1)) {
        // nothing to read, spurious interrupt
        return -2;
    }
    if (kBytesRead == MAX_KBUF_SIZE) {
        // discard characters because buffer is full
        kprintf("KEYBOARD BUFFER FULL\n");
        return -3;
    }

    if (parsedCharacter == NOCHAR) {
        // discard uneeded scan codes
        return -4;
    }
    char character = (char) parsedCharacter;
    if (TOGGLE_ECHO) {
        kprintf("%c", character);
    }
    // Put it in the buffer
    if (kBytesRead < MAX_KBUF_SIZE) {
        kbuf[kBytesRead] = character;
        kBytesRead++;
    }
    if (kbDataRequest.status) {
        char * buff = kbDataRequest.buff;
        int bytesRead = kbDataRequest.bytesRead;
        int size = kbDataRequest.size;
        // Copy as much from the buffer into the dataRequest
        bytesRead = copyCharactersToBuffer(buff, size, bytesRead, &kbuf[0], MAX_KBUF_SIZE, &kBytesRead);
        if (bytesRead == size || character == '\n') {
            kbDataRequest.done(bytesRead);
        } else if (state == INCTL && character == EOFINDICATOR) {
            kprintf("CTRL-D/EOF DETECTED!!!\n");

        // Disable interrupts
        //enable_irq(1,1);
        }
    }
    return 0;
}

int copyCharactersToBuffer(char *outBuf, int outBufSize, int outBufBytesRead, char * inBuf, int inBufSize, int *inBufBytesRead) {
    int bytesCopied = 0;
    while (outBufBytesRead < outBufSize && bytesCopied < *inBufBytesRead) {
        outBuf[outBufSize] = inBuf[bytesCopied];
        outBufBytesRead++;
        bytesCopied++;
    }
    *inBufBytesRead -= bytesCopied;
    return outBufBytesRead;
}


static int extchar(unsigned char code) {
    return state &= ~EXTENDED;
}


unsigned int kbtoa( unsigned char code )
{
    unsigned int ch;

    if (state & EXTENDED)
        return extchar(code);

    if (code & KEY_UP) {
        switch (code & 0x7f) {
            case LSHIFT:
            case RSHIFT:
                state &= ~INSHIFT;
                break;
            case CAPSL:
                kprintf("Capslock off detected\n");
                state &= ~CAPSLOCK;
                break;
            case LCTL:
                state &= ~INCTL;
                break;
            case LMETA:
                state &= ~INMETA;
                break;
        }

        return NOCHAR;
    }


    /* check for special keys */
    switch (code) {
        case LSHIFT:
        case RSHIFT:
            state |= INSHIFT;
            //kprintf("shift detected!\n");
            return NOCHAR;
        case CAPSL:
            state |= CAPSLOCK;
            kprintf("Capslock ON detected!\n");
            return NOCHAR;
        case LCTL:
            state |= INCTL;
            kprintf("CTRL detected!\n");
            return NOCHAR;
        case LMETA:
            state |= INMETA;
            return NOCHAR;
        case EXTESC:
            state |= EXTENDED;
            return NOCHAR;
    }

    ch = NOCHAR;

    if (code < sizeof(kbcode)){
        if ( state & CAPSLOCK )
            ch = kbshift[code];
        else
            ch = kbcode[code];
    }
    if (state & INSHIFT) {
        if (code >= sizeof(kbshift))
            return NOCHAR;
        if ( state & CAPSLOCK )
            ch = kbcode[code];
        else
            ch = kbshift[code];
    }
    if (state & INCTL) {
        if (code >= sizeof(kbctl))
            return NOCHAR;
        ch = kbctl[code];
    }
    if (state & INMETA)
        ch += 0x80;
    return ch;
}



//===============================================
//   HELPER FUNCTIONS (NOT USING CURRENTLY)
//===============================================

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ready
 *  Description:  adds a pcb process to given list identified by head and tail.
 *       Return:  1 if sucessful 0 if unsucessful
 * =====================================================================================
 */
int queueRequest(struct dataRequest *dr, struct dataRequest **head, struct dataRequest **tail, int status){
    if(!dr){
        return -1;
    }
    if(!*head && !*tail){
        *head = dr;
        *tail = dr;
        dr->prev = NULL;
        dr->next = NULL;
        dr->status = status;
        return 1;
    }
    if(*head && *tail){
        (*tail)->next = dr;
        dr->prev = *tail;
        *tail = dr;
        (*tail)->next = NULL;
        dr->status = status;
        return 1;
    }
    kprintf("\n\n one of QueueHead or QueueTail is NULL\n file: disp.c\n function: ready");
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  next
 *  Description:  removes a pcb struct from given list identified by the given head.
 *       Return:  return pcb struct, null if non availble in given list
 * =====================================================================================
 */
struct dataRequest* nextRequest(struct dataRequest **head, struct dataRequest **tail){
    if(!*head){
        // gets idle process only if dealing with ready queue
        return NULL;
    }
    //if they are the same only one thing on list
    if (*head == *tail) {
        *tail = NULL;
    }
    struct dataRequest *nextDr = *head;
    *head = (*head)->next;
    nextDr->next = NULL;
    nextDr->prev = NULL;
    (*head)->prev = NULL;
    return nextDr;
}




