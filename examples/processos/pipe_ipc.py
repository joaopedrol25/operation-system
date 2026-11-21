import os
import sys


def enviar(saida: int) -> None:
    mensagem = "Olá pelo pipe!".encode("utf-8")
    enviados = 0
    while enviados < len(mensagem):
        escritos = os.write(saida, mensagem[enviados:])
        if escritos == 0:
            raise OSError("nenhum byte enviado")
        enviados += escritos


def receber(entrada: int) -> str:
    mensagem = bytearray()
    while True:
        bloco = os.read(entrada, 8)
        if not bloco:  # Uma leitura vazia indica EOF.
            break
        mensagem.extend(bloco)
    # Decodifique depois de juntar todos os bytes.
    return mensagem.decode("utf-8")


def main() -> int:
    try:
        entrada, saida = os.pipe()
    except OSError as erro:
        print(f"pipe: {erro}", file=sys.stderr)
        return 1

    try:
        filho = os.fork()
    except OSError as erro:
        os.close(entrada)
        os.close(saida)
        print(f"fork: {erro}", file=sys.stderr)
        return 1

    if filho == 0:
        os.close(entrada)
        resultado = 0
        try:
            enviar(saida)
        except OSError as erro:
            print(f"write: {erro}", file=sys.stderr, flush=True)
            resultado = 1
        finally:
            os.close(saida)
        os._exit(resultado)

    os.close(saida)
    try:
        mensagem = receber(entrada)
    except (OSError, UnicodeDecodeError) as erro:
        print(f"read: {erro}", file=sys.stderr)
        mensagem = None
    finally:
        os.close(entrada)

    try:
        _, estado = os.waitpid(filho, 0)
    except OSError as erro:
        print(f"waitpid: {erro}", file=sys.stderr)
        return 1

    if not os.WIFEXITED(estado) or os.WEXITSTATUS(estado) != 0:
        print("O filho falhou ao enviar a mensagem.", file=sys.stderr)
        return 1

    if mensagem is None:
        return 1

    print(f'O pai recebeu "{mensagem}" antes do fim do pipe.')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
