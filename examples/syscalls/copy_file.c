#define _POSIX_C_SOURCE 200809L
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void report_error(const char *action, const char *path)
{
    int error = errno;
    fprintf(stderr, "%s '%s': %s\n", action, path, strerror(error));
}

static int copy_bytes(int input, int output, const char *source, const char *destination)
{
    unsigned char buffer[16 * 1024];

    for (;;) {
        ssize_t bytes_read;
        do {
            bytes_read = read(input, buffer, sizeof(buffer));
        } while (bytes_read == -1 && errno == EINTR);

        if (bytes_read == -1) {
            report_error("Erro ao ler", source);
            return EXIT_FAILURE;
        }
        if (bytes_read == 0) {
            return EXIT_SUCCESS;
        }

        /* write() pode gravar apenas uma parte dos bytes solicitados. */
        size_t written = 0;
        while (written < (size_t)bytes_read) {
            ssize_t count = write(output, buffer + written, (size_t)bytes_read - written);
            if (count == -1 && errno == EINTR) {
                continue;
            }
            if (count <= 0) {
                if (count == 0) {
                    errno = EIO;
                }
                report_error("Erro ao gravar", destination);
                return EXIT_FAILURE;
            }
            written += (size_t)count;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s ORIGEM DESTINO\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *source = argv[1];
    const char *destination = argv[2];
    int input;
    do {
        /* O_NONBLOCK evita esperar por um escritor se a origem for um FIFO. */
        input = open(source, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    } while (input == -1 && errno == EINTR);
    if (input == -1) {
        report_error("Erro ao abrir a origem", source);
        return EXIT_FAILURE;
    }

    struct stat info;
    if (fstat(input, &info) == -1) {
        report_error("Erro ao consultar a origem", source);
        close(input);
        return EXIT_FAILURE;
    }
    if (!S_ISREG(info.st_mode)) {
        fprintf(stderr, "A origem '%s' precisa ser um arquivo regular.\n", source);
        close(input);
        return EXIT_FAILURE;
    }

    int output;
    do {
        /* O_EXCL recusa qualquer nome existente, inclusive um link para a origem. */
        output = open(destination, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0666);
    } while (output == -1 && errno == EINTR);
    if (output == -1) {
        report_error("Erro ao criar o destino", destination);
        close(input);
        return EXIT_FAILURE;
    }

    int result = copy_bytes(input, output, source, destination);

    /* No Linux, close() pode liberar o descritor mesmo quando retorna erro. */
    if (close(output) == -1) {
        report_error("Erro ao fechar o destino", destination);
        result = EXIT_FAILURE;
    }
    if (close(input) == -1) {
        report_error("Erro ao fechar a origem", source);
        result = EXIT_FAILURE;
    }
    /* O chamador controla o diretório de destino durante a cópia. */
    if (result == EXIT_FAILURE && unlink(destination) == -1) {
        report_error("Erro ao remover a cópia parcial", destination);
    }

    return result;
}
