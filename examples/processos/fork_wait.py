import os
import sys


CODIGO_DO_FILHO = 7


def main() -> int:
    valor = 10

    try:
        filho = os.fork()
    except OSError as erro:
        print(f"fork: {erro}", file=sys.stderr)
        return 1

    if filho == 0:
        valor = 20
        try:
            # flush=True grava a saída antes de chamar os._exit().
            print(f"O filho alterou sua cópia do valor para {valor}.", flush=True)
        except OSError as erro:
            print(f"stdout: {erro}", file=sys.stderr, flush=True)
            os._exit(1)
        os._exit(CODIGO_DO_FILHO)

    try:
        _, estado = os.waitpid(filho, 0)
    except OSError as erro:
        print(f"waitpid: {erro}", file=sys.stderr)
        return 1

    if not os.WIFEXITED(estado):
        print("O filho foi interrompido antes de concluir.", file=sys.stderr)
        return 1

    codigo = os.WEXITSTATUS(estado)
    print(f"O pai ainda tem o valor {valor}.")
    print(f"O filho terminou com código {codigo}.")
    return 0 if codigo == CODIGO_DO_FILHO else 1


if __name__ == "__main__":
    raise SystemExit(main())
