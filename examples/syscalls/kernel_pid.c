#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 1) {
        fprintf(stderr, "Uso: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* A libc oferece o wrapper específico e a função genérica syscall(). */
    pid_t libc_pid = getpid();
    long kernel_pid = syscall(SYS_getpid);
    if (kernel_pid == -1) {
        perror("syscall(SYS_getpid)");
        return EXIT_FAILURE;
    }

    printf("PID via libc getpid(): %ld\n", (long)libc_pid);
    printf("PID via syscall(SYS_getpid): %ld\n", kernel_pid);

    if ((long)libc_pid != kernel_pid) {
        fprintf(stderr, "As duas chamadas retornaram PIDs diferentes.\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
