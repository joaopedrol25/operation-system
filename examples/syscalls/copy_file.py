import os
import stat
import sys


def copy_bytes(source: str, destination: str) -> None:
    created = False
    try:
        # As funções de os expõem operações POSIX via Python.
        # O_NONBLOCK evita esperar pela outra ponta de um FIFO.
        input_fd = os.open(source, os.O_RDONLY | os.O_NONBLOCK)
        try:
            if not stat.S_ISREG(os.fstat(input_fd).st_mode):
                raise OSError(f"A origem '{source}' precisa ser um arquivo regular.")

            # O_EXCL reserva o nome do destino em uma operação, sem sobrescrever.
            output_fd = os.open(destination, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o666)
            created = True
            try:
                # Python repete read() e write() após EINTR se o manipulador retornar normalmente.
                while data := os.read(input_fd, 16 * 1024):
                    pending = memoryview(data)
                    while pending:
                        written = os.write(output_fd, pending)
                        if written == 0:
                            raise OSError("A escrita no destino não avançou.")
                        pending = pending[written:]
            finally:
                os.close(output_fd)
        finally:
            os.close(input_fd)
    except OSError:
        # O chamador controla o diretório de destino durante a cópia.
        if created:
            try:
                os.unlink(destination)
            except FileNotFoundError:
                pass
            except OSError as error:
                print(f"Erro ao remover a cópia parcial: {error}", file=sys.stderr)
        raise


def main() -> int:
    if len(sys.argv) != 3:
        print(f"Uso: {sys.argv[0]} ORIGEM DESTINO", file=sys.stderr)
        return 1

    try:
        copy_bytes(sys.argv[1], sys.argv[2])
    except OSError as error:
        print(f"Erro ao copiar: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
