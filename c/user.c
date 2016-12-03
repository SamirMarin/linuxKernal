/* user.c : User processes
*/

#include <xeroskernel.h>
#include <xeroslib.h>

#define BUF_MAX 100
char *username = "cs415\n";
char *password = "EveryoneGetsAnA\n";
char *pscom = "ps";
char *excom = "ex";
char *kcom = "k";
char *acom = "a";
char *tcom = "t";
char *psand = "ps&";
char *exand = "ex&";
char *kand = "k&";
char *aand = "a&";
char *tand = "t&";
int shellPid;
int alarmTicks;


void shell(void);
void  root(void);
void psf(void);
void exf(void);
void kf(int pid);
void alarmHandler(void *cntx);
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
        stdinput[bytes++] = NULLCH;
        char command[bytes];
        int bytesParsed = parseString(stdinput, bytes, command, bytes);
        if (bytesParsed == -2) {
            // GO back to the the beginning of the loop
            sysputs("Ignoring command");
            continue;
        }
        command[bytesParsed++] = NULLCH;
        if (!strcmp(command, pscom) || !strcmp(command, psand)) {
                psf();
        } else if (!strcmp(command, excom) || !strcmp(command, exand)) {
            break;
        } else if (!strcmp(command, kcom) || !strcmp(command, kand)) {
            if (bytesParsed < BUF_MAX) {
                char arg[BUF_MAX];
                bytesParsed += parseString(&stdinput[bytesParsed], BUF_MAX - bytesParsed, arg, BUF_MAX);
                arg[bytesParsed++] = NULLCH;
                int pid = atoi(arg);
                int res = syskill(pid, 9);
                if (res == -712) {
                    sprintf(arg, "No such process\n");
                    sysputs(arg);
                }
            }
        } else if (!strcmp(command, acom) || !strcmp(command, aand)) {
            if (bytesParsed < BUF_MAX) {
                char arg[BUF_MAX];
                bytesParsed += parseString(&stdinput[bytesParsed], BUF_MAX - bytesParsed, arg, BUF_MAX);
                arg[bytesParsed++] = NULLCH;
                int ticks = atoi(arg);
                alarmTicks = ticks;
                syssighandler(15, &alarmHandler, NULL);
                int alarmPid = syscreate(&alarm, 8000);
                if (!strcmp(command, acom)) {
                    syswait(alarmPid);
                }
            }
        } else if (!strcmp(command, tcom) || !strcmp(command, tand)) {
            int tpid = syscreate(&t, 8000);
            if (!strcmp(command, tcom)) {
                syswait(tpid);
            }
        } else {
            sysputs("Command not found\n");
        }
    }
    sysputs("Exiting shell...\n");
}

int parseString(char *inBuf, int inBufSize, char *outBuf, int outBufSize) {
        int bytesRead = 0;
        char * endInBuf = inBuf + inBufSize;
        char * endOutBuf = outBuf + outBufSize;
        while (inBuf < endInBuf && *inBuf == ' ') {
            inBuf++;
        }
        while (inBuf < endInBuf && *inBuf != ' ' && *inBuf != '\n' && outBuf < endOutBuf ) {
            *outBuf = *inBuf;
            outBuf++;
            inBuf++;
            bytesRead++;
        }
        return bytesRead;
}

void psf(void) {
    struct processStatuses ps;
    int procs =  sysgetcputimes(&ps);
    char buf[100];
    sprintf(buf, "%4s    %4s    %10s\n", "Pid", "Status", "CpuTime");
    sysputs(buf);
    for (int i = 0; i <= procs; i++) {
        int status = ps.status[i];
        switch(status) {
        case STATE_STOPPED:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "STOPPED", ps.cpuTime[i]);
            break;
        case STATE_READY:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "READY", ps.cpuTime[i]);
            break;
        case STATE_SLEEP:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "SLEEP", ps.cpuTime[i]);
            break;
        case STATE_RUNNING:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "RUNNING", ps.cpuTime[i]);
            break;
        case STATE_RECV:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "RECV", ps.cpuTime[i]);
            break;
        case STATE_SEND:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "SENDING", ps.cpuTime[i]);
            break;
        case STATE_WAITING:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "WAITING", ps.cpuTime[i]);
            break;
        case STATE_DEV_WAITING:
            sprintf(buf, "%4d    %4s    %10d\n", ps.pid[i], "DEV_WAITING", ps.cpuTime[i]);
            break;
        }
        sysputs(buf);
    }
}
void kf(int pid) {}

void alarmHandler(void *cntx) {
    char buf[100];
    sprintf(buf, "ALARM ALARM ALARM\n");
    sysputs(buf);
    syssighandler(15, NULL, NULL);
}

void alarm(void) {
    int sleepTime = alarmTicks * TICKLENGTH;
    syssighandler(9, &sysstop, NULL);
    syssleep(sleepTime);
    syskill(shellPid, 15); 
}

void t(void) {
    char buf[5];
    syssighandler(9, &sysstop, NULL);
    sprintf(buf, "T\n");
    for (;;) {
        syssleep(10000);
        sysputs(buf);
    }
}
