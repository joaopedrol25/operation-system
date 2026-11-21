#define _POSIX_C_SOURCE 200809L
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static bool parse_offset(const char *text, off_t *offset)
{
    if (text[0] == '\0') {
        return false;
    }
    for (const char *character = text; *character != '\0'; ++character) {
        if (*character < '0' || *character > '9') {
            return false;
        }
    }

    errno = 0;
    intmax_t value = strtoimax(text, NULL, 10);
    if (errno == ERANGE || (intmax_t)(off_t)value != value) {
        return false;
    }
    *offset = (off_t)value;
    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s ARQUIVO DESLOCAMENTO\n", argv[0]);
        return EXIT_FAILURE;
    }

    off_t offset;
    if (!parse_offset(argv[2], &offset)) {
        fprintf(stderr, "DESLOCAMENTO precisa ser um inteiro decimal não negativo dentro do limite do sistema.\n");
        return EXIT_FAILURE;
    }

    int file;
    do {
        file = open(argv[1], O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    } while (file == -1 && errno == EINTR);
    if (file == -1) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    /* SEEK_SET mede a posição desde o início, em bytes. */
    off_t position = lseek(file, offset, SEEK_SET);
    if (position == (off_t)-1) {
        if (errno == ESPIPE) {
            fprintf(stderr, "A entrada é um pipe ou outro fluxo sem suporte a lseek.\n");
        } else {
            perror("Erro em lseek");
        }
        close(file);
        return EXIT_FAILURE;
    }

    /* read() consome um byte na posição atual e avança o cursor. */
    unsigned char byte;
    ssize_t count;
    do {
        count = read(file, &byte, sizeof(byte));
    } while (count == -1 && errno == EINTR);
    if (count <= 0) {
        if (count == 0) {
            fprintf(stderr, "Fim do arquivo na posição %" PRIdMAX ".\n", (intmax_t)position);
        } else {
            perror("Erro ao ler o arquivo");
        }
        close(file);
        return EXIT_FAILURE;
    }
    if (close(file) == -1) {
        perror("Erro ao fechar o arquivo");
        return EXIT_FAILURE;
    }

    printf("Byte na posição %" PRIdMAX ": 0x%02X\n", (intmax_t)position, (unsigned int)byte);
    return EXIT_SUCCESS;
}
