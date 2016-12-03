/* user.c : User processes
*/

#include <xeroskernel.h>
#include <xeroslib.h>

#define BUF_MAX 100
char *username = "cs415\n";
char *password = "EveryoneGetsAnA\n";
char *ps = "ps\n";
char *ex = "ex\n";
char *k = "k\n";


void shell(void);
void  root(void);
int parseString(char *inBuf, int inBufSize, char *outBuf, int outBufSize);

void  root( void ) {
    int error = 0;
    char ubuf[BUF_MAX];
    char pbuf[BUF_MAX];

    while (1) {
        // Banner
        sysputs("\nWelcome to Xeros - an experimental OS\n");

        // Open keyboard in non echo mode
        int fd = sysopen(0);
        if (fd == -1) {
            kprintf("Error opening keyboard\n");
            for(;;);
        }

        // Turn keyboard echoing on;
        error = sysioctl(fd, 56);
        if (error == -1) {
            kprintf("Error turning keyboard echoing on\n");
            for(;;);
        }

        sysputs("Username: ");
        int bytes = sysread(fd, &ubuf[0], BUF_MAX);
        if (!bytes) {
            kprintf("Sysread returned EOF\n");
            for(;;);
        }
        if (bytes == -1) {
            kprintf("Sysread returned an error\n");
            for(;;);
        }
        ubuf[bytes] = NULLCH;
        // Turn keyboard echoing off;
        error = sysioctl(fd, 55);
        if (error == -1) {
            kprintf("Error turning keyboard echoing off\n");
            for(;;);
        }
        sysputs("Password: ");
        bytes = sysread(fd, &pbuf[0], BUF_MAX);
        if (!bytes) {
            kprintf("Sysread returned EOF\n");
            for(;;);
        }
        if (bytes == -1) {
            kprintf("Sysread returned an error\n");
            for(;;);
        }
        pbuf[bytes] = NULLCH;
        error = sysclose(fd); // Just writing this in for testing even though we dont actually have to close the fd
        if (error == -1) {
            kprintf("Error turning closing device\n");
            for(;;);
        }
        int usercheck = strcmp(&ubuf[0], username);
        int passcheck = strcmp(&pbuf[0], password);
        //kprintf("\n user check %d, pass check %d", usercheck, passcheck);
        //kprintf("\n user in %s, pass in %s", ubuf, pbuf);
        if (strcmp(ubuf, username) == 0 && strcmp(pbuf, password) == 0) {
            break;
        }
    }
    char buf[BUF_MAX];
    sprintf(buf, "\n");
    sysputs(buf);
    
    int shellPid = create(&shell, 8000);
    int retCode = syswait(shellPid);
    sprintf(buf, "Syswait retcode%d\n", retCode);
    sysputs(buf);

}

void shell(void) {
    char stdinput[BUF_MAX];
    // Open keyboard in echo mode
    int fd = sysopen(1);
    if (fd == -1) {
        kprintf("Error opening keyboard\n");
        for(;;);
    }

    while (1) {
        sysputs("> ");
        int bytes = sysread(fd, &stdinput[0], BUF_MAX);
        if (!bytes) {
            kprintf("Sysread returned EOF\n");
            for(;;);
        }
        if (bytes == -1) {
            kprintf("Sysread returned an error\n");
            for(;;);
        }
        if (bytes == BUF_MAX) {
            bytes = BUF_MAX - 1;
        }
        stdinput[bytes] = NULLCH;
        char parsedWord[bytes];
        int bytesParsed = parseString(stdinput, bytes, parsedWord, bytes);
        if (bytesParsed == -2) {
            // GO back to the the beginning of the loop
            sysputs("Ignoring command");
            continue;
        }
        parsedWord[bytesParsed] = NULLCH;
        sysputs(parsedWord);
    }
}

int parseString(char *inBuf, int inBufSize, char *outBuf, int outBufSize) {
        int bytesRead = 0;
        char * endInBuf = inBuf + inBufSize;
        char * endOutBuf = outBuf + outBufSize;
        while (inBuf < endInBuf && *inBuf == ' ') {
            inBuf++;
        }
        while (inBuf < endInBuf && *inBuf != ' ' && outBuf < endOutBuf ) {
            *outBuf++ = *inBuf++;
            bytesRead++;
        }
        if (inBuf[1] == '&'){
            return -2;
        }
        return bytesRead;
}

