#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t filho = fork();
    if (filho == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (filho == 0) {
        /* Em caso de sucesso, echo assume o lugar do programa no processo filho. */
        execlp("echo", "echo", "O filho agora executa o comando echo.", (char *)NULL);
        perror("execlp");
        _exit(127);
    }

    int estado;
    pid_t terminado;
    do {
        terminado = waitpid(filho, &estado, 0);
    } while (terminado == -1 && errno == EINTR);

    if (terminado == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(estado)) {
        int codigo = WEXITSTATUS(estado);
        printf("O pai recebeu o código de saída %d.\n", codigo);
        return codigo;
    }

    if (WIFSIGNALED(estado)) {
        int sinal = WTERMSIG(estado);
        fprintf(stderr, "O comando terminou pelo sinal %d.\n", sinal);
        return 128 + sinal;
    }

    return EXIT_FAILURE;
}
