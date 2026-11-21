import errno
import os
import sys


def parse_offset(text: str) -> int:
    message = "DESLOCAMENTO precisa ser um inteiro decimal não negativo dentro do limite do sistema."
    if not (text.isascii() and text.isdecimal()):
        raise ValueError(message)

    digits = text.lstrip("0") or "0"
    if len(digits) > 19:
        raise ValueError(message)
    offset = int(digits)
    if offset > (1 << 63) - 1:
        raise ValueError(message)
    return offset


def main() -> int:
    if len(sys.argv) != 3:
        print(f"Uso: {sys.argv[0]} ARQUIVO DESLOCAMENTO", file=sys.stderr)
        return 1

    try:
        offset = parse_offset(sys.argv[2])
    except ValueError as error:
        print(error, file=sys.stderr)
        return 1

    try:
        fd = os.open(sys.argv[1], os.O_RDONLY | os.O_NONBLOCK)
        try:
            # os.lseek() expõe a operação POSIX que move a posição do descritor.
            position = os.lseek(fd, offset, os.SEEK_SET)
            byte = os.read(fd, 1)
        finally:
            os.close(fd)
    except OSError as error:
        if error.errno == errno.ESPIPE:
            print("A entrada é um pipe ou outro fluxo sem suporte a lseek.", file=sys.stderr)
        else:
            print(f"Erro ao acessar o arquivo: {error}", file=sys.stderr)
        return 1
    if not byte:
        print(f"Fim do arquivo na posição {position}.", file=sys.stderr)
        return 1

    print(f"Byte na posição {position}: 0x{byte[0]:02X}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
