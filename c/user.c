/* user.c : User processes
*/

#include <xeroskernel.h>
#include <xeroslib.h>

#define BUF_MAX 100
char *username = "cs415\n";
char *password = "EveryoneGetsAnA\n";
char *pscom = "ps\n";
char *excom = "ex\n";
char *kcom = "k\n";
char *acom = "a\n";
char *tcom = "t\n";
int shellPid;
int alarmTicks;


void shell(void);
void  root(void);
void psf(void);
void exf(void);
void kf(int pid);
void alarmHandler(void);
void alarm(void);
void t(void);
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
        int bytes = sysread(fd, &ubuf[0], BUF_MAX - 1);
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
        bytes = sysread(fd, &pbuf[0], BUF_MAX - 1);
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
        if (usercheck == 0 && passcheck == 0) {
            break;
        }
    }
    char buf[BUF_MAX];
    sprintf(buf, "\n");
    sysputs(buf);
    
    shellPid = create(&shell, 8000);
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
        int bytes = sysread(fd, &stdinput[0], BUF_MAX - 1);
        if (!bytes) {
            break;
        }
        if (bytes == -1) {
            kprintf("Sysread returned an error\n");
            for(;;);
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
        if (strcmp(parsedWord, pscom) == 0) {
                psf();

        } else if (strcmp(parsedWord, excom) == 0) {
            break;

        } else if (strcmp(parsedWord, kcom) == 0) {
            if (bytes < BUF_MAX) {
                char arg[BUF_MAX];
                int bytesParsed = parseString(&stdinput[bytes], BUF_MAX, arg, BUF_MAX);
                arg[bytesParsed] = NULLCH;
                int pid = atoi(arg);
                kprintf("PID: %d\n", pid);
            }
        } else if (strcmp(parsedWord, acom) == 0) {
            if (bytes < BUF_MAX) {
                char arg[BUF_MAX];
                int bytesParsed = parseString(&stdinput[bytes], BUF_MAX, arg, BUF_MAX);
                int ticks = atoi(arg);
                arg[bytesParsed] = NULLCH;
                kprintf("TICKS: %d\n", ticks);
                alarmTicks = ticks;
                syssighandler(15, &alarmHandler, NULL);
                syscreate(&alarm, 8000);
            }

        } else if (strcmp(parsedWord, tcom) == 0) {
            syscreate(&t, 8000);
        } else {
            sysputs("Command not found\n");
        }
        sysputs(parsedWord);
    }
    sysputs("Exiting shell...\n");
}

int parseString(char *inBuf, int inBufSize, char *outBuf, int outBufSize) {
        int bytesRead = 0;
        char * endInBuf = inBuf + inBufSize;
        char * endOutBuf = outBuf + outBufSize;
        while (inBuf < endInBuf && (*inBuf == ' ' || *inBuf == NULLCH)) {
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

void psf(void) {
    struct processStatuses ps;
    int procs =  sysgetcputimes(&ps);
    char buf[100];
    sprintf(buf, "%4s    %4s    %10s\n", "Pid", "Status", "CpuTime");
    for (int i = 0; i < procs; i++) {
        sprintf(buf, "%4d    %4d    %10d\n", ps.pid[i], ps.status[i], ps.cpuTime[i]);
    }
}
void kf(int pid) {}

void alarmHandler(void) {
    char buf[100];
    sprintf(buf, "ALARM ALARM ALARM\n");
    sysputs(buf);
    syssighandler(15, NULL, NULL);
}

void alarm(void) {
    int sleepTime = alarmTicks * TICKLENGTH;
    syssleep(sleepTime);
    signal(shellPid, 15); 
}

void t(void) {
    char buf[5];
    sprintf(buf, "T\n");
    for (;;) {
        syssleep(10000);
        sysputs(buf);
    }
}
