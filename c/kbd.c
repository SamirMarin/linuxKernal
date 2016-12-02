#include <xeroskernel.h>
#include <kbd.h>


char keybuf[4];
int  buffBytes;

char ENABLE_ECHO = 0;
char ECHO_IN_USE = 0;
char NON_ECHO_IN_USE = 0;

struct request* reqHead;
struct request* reqTail;
static int state; /* the state of the keyboard */

unsigned int kbtoa( unsigned char code );


int ekbd_read(void * buff, int size) {


    return 0;
}


void done(struct request *req) {



}


// ==================================
//            LOWER HALF
// ==================================
int kbd_close(struct devsw* dblock) {

    // disable hardware
return 0;
}


int kbd_read_in() {
    unsigned char ctrlByte = inb(CTRL_PORT);
    if (!(ctrlByte & 1)) {
        // nothing to read, spurious interrupt
        return -1;
    }
    unsigned long scanCode = inb(READ_PORT);
    char character = kbtoa(scanCode);
    kprintf("Scan code: %x, Character: %c\n", scanCode, character);

    if (!reqHead) {
        // put it in local buffer
    } else {
        // put it in the request buffer


    }

    return 0;
}








unsigned char   code;
static int extchar(code)
{
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
    kprintf("shift detected!\n");
    return NOCHAR;
  case CAPSL:
    state |= CAPSLOCK;
    kprintf("Capslock ON detected!\n");
    return NOCHAR;
  case LCTL:
    state |= INCTL;
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



//===================================
//          HELPER FUNCTIONS
//===================================

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ready
 *  Description:  adds a pcb process to given list identified by head and tail.
 *       Return:  1 if sucessful 0 if unsucessful
 * =====================================================================================
 */
int queueRequest(struct pcb *process, struct pcb **head, struct pcb **tail, int state){
    if(!process){
        return -1;
    }
    if(process == idleProcessHead){
        return -2;
    }
    if(!*head && !*tail){
        *head = process;
        *tail = process;
        process->prev = NULL;
        process->next = NULL;
        process->head = head;
        process->tail = tail;
        process->state = state;
        return 1;
    }
    if(*head && *tail){
        (*tail)->next = process;
        process->prev = *tail;
        *tail = process;
        process->head = head;
        process->tail = tail;
        (*tail)->next = NULL;
        process->state = state;
        return 1;
    }
    kprintf("\n\n one of QueueHead or QueueTail is NULL\n file: disp.c\n function: ready");
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  next
 *  Description:  removes a pcp struct from given list identified by the given head.
 *       Return:  return pcp struct, null if non availble in given list
 * =====================================================================================
 */
struct request* nextRequest(struct request **head, struct request **tail){
    if(!*head){
        // gets idle process only if dealing with ready queue
        return NULL;
    }
    //if they are the same only one thing on list
    if (*head == *tail) {
        *tail = NULL;
    }
    struct pcb *nextProcess = *head;
    *head = (*head)->next;
    nextProcess->next = NULL;
    nextProcess->prev = NULL;
    (*head)->prev = NULL;
    nextProcess->head = NULL;
    nextProcess->tail = NULL;
    return nextProcess;
}




