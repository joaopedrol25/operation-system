#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    enum { CODIGO_DO_FILHO = 7 };
    int valor = 10;

    /* O buffer de saída começa vazio nos dois processos. */
    pid_t filho = fork();
    if (filho == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (filho == 0) {
        valor = 20;
        /* _exit encerra o processo imediatamente, então gravamos a saída antes. */
        if (printf("O filho alterou sua cópia do valor para %d.\n", valor) < 0
            || fflush(stdout) == EOF) {
            perror("stdout");
            _exit(EXIT_FAILURE);
        }
        _exit(CODIGO_DO_FILHO);
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

    if (!WIFEXITED(estado)) {
        fprintf(stderr, "O filho foi interrompido antes de concluir.\n");
        return EXIT_FAILURE;
    }

    int codigo = WEXITSTATUS(estado);
    printf("O pai ainda tem o valor %d.\n", valor);
    printf("O filho terminou com código %d.\n", codigo);
    return codigo == CODIGO_DO_FILHO ? EXIT_SUCCESS : EXIT_FAILURE;
}
