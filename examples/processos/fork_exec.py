import os
import sys


def main() -> int:
    try:
        filho = os.fork()
    except OSError as erro:
        print(f"fork: {erro}", file=sys.stderr)
        return 1

    if filho == 0:
        try:
            # Em caso de sucesso, echo substitui o interpretador Python no filho.
            os.execlp("echo", "echo", "O filho agora executa o comando echo.")
        except OSError as erro:
            print(f"execlp: {erro}", file=sys.stderr, flush=True)
            os._exit(127)

    try:
        _, estado = os.waitpid(filho, 0)
    except OSError as erro:
        print(f"waitpid: {erro}", file=sys.stderr)
        return 1

    if os.WIFEXITED(estado):
        codigo = os.WEXITSTATUS(estado)
        print(f"O pai recebeu o código de saída {codigo}.")
        return codigo

    if os.WIFSIGNALED(estado):
        sinal = os.WTERMSIG(estado)
        print(f"O comando terminou pelo sinal {sinal}.", file=sys.stderr)
        return 128 + sinal

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
