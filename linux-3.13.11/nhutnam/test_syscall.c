#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#if defined(__x86_64__)
#define pnametoid 314
#define pidtoname 315
#elif defined(__i386__)
#define pnametoid 351
#define pidtoname 352
#else
#define pnametoid 274
#define pidtoname 275
#endif

int main() {
    printf("call pnametoid (%d)\n", pnametoid);
    printf("pnametoid return %d\n", (int)syscall(pnametoid, (char *)"firefox"));
    printf("call pidtoname (%d)\n", pidtoname);
    char buf[256];
    buf[0] = '\0';
    printf("pidtoname return %d\n", (int)syscall(pidtoname, (int)getpid(), (char *)buf, (int)256));
    printf("process name: %s\n", buf);
    return 0;
}
