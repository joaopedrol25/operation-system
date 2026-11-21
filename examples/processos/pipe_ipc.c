#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int escrever_mensagem(int saida)
{
    const char mensagem[] = "Olá pelo pipe!";
    const size_t tamanho = sizeof mensagem - 1;
    size_t enviados = 0;

    while (enviados < tamanho) {
        ssize_t n = write(saida, mensagem + enviados, tamanho - enviados);
        if (n == -1 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            if (n == 0) {
                errno = EIO;
            }
            perror("write");
            return EXIT_FAILURE;
        }
        enviados += (size_t)n;
    }

    return EXIT_SUCCESS;
}

static int ler_mensagem(int entrada)
{
    char bloco[8];

    if (fputs("O pai recebeu \"", stdout) == EOF) {
        perror("stdout");
        return EXIT_FAILURE;
    }

    for (;;) {
        ssize_t n = read(entrada, bloco, sizeof bloco);
        if (n == 0) {
            break; /* Todas as pontas de escrita foram fechadas. */
        }
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            return EXIT_FAILURE;
        }

        /* Mesmo uma leitura curta pode trazer parte da mensagem. */
        if (fwrite(bloco, 1, (size_t)n, stdout) != (size_t)n) {
            perror("stdout");
            return EXIT_FAILURE;
        }
    }

    if (puts("\" antes do fim do pipe.") == EOF || fflush(stdout) == EOF) {
        perror("stdout");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(void)
{
    int pontas[2];
    if (pipe(pontas) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t filho = fork();
    if (filho == -1) {
        perror("fork");
        close(pontas[0]);
        close(pontas[1]);
        return EXIT_FAILURE;
    }

    if (filho == 0) {
        close(pontas[0]);
        int resultado = escrever_mensagem(pontas[1]);
        close(pontas[1]);
        _exit(resultado);
    }

    close(pontas[1]);
    int resultado = ler_mensagem(pontas[0]);
    close(pontas[0]);

    int estado;
    pid_t terminado;
    do {
        terminado = waitpid(filho, &estado, 0);
    } while (terminado == -1 && errno == EINTR);

    if (terminado == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(estado) || WEXITSTATUS(estado) != EXIT_SUCCESS) {
        fprintf(stderr, "O filho falhou ao enviar a mensagem.\n");
        return EXIT_FAILURE;
    }

    return resultado;
}
