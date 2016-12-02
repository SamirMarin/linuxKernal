/* user.c : User processes
*/

#include <xeroskernel.h>
#include <xeroslib.h>

#define BUF_MAX 36
char *username = "cs415\n";
char *password = "EveryoneGetsAnA";

void shell(void);
void  root(void);

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
        error = sysioctl(fd, 55);
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
        error = sysioctl(fd, 56);
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
        if (strcmp(ubuf, username) == 0 && strcmp(pbuf, password) == 0) {
            break;
        }
    }

    int shellPid = create(&shell, 8000);
    syswait(shellPid);

}

void shell(void) {
    int stdinput[BUF_MAX];
    // Open keyboard in non echo mode
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


        

    }



}

