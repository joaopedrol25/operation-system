import os
from pathlib import Path
import sys


def main() -> int:
    if len(sys.argv) != 1:
        print(f"Uso: {sys.argv[0]}", file=sys.stderr)
        return 1

    # os.getpid() é uma API de Python. O procfs expõe o PID como texto.
    python_pid = os.getpid()
    try:
        proc_pid = int(Path("/proc/self/stat").read_bytes().partition(b" ")[0])
    except (OSError, ValueError) as error:
        print(f"Erro ao consultar /proc/self/stat: {error}", file=sys.stderr)
        return 1

    print(f"PID via os.getpid(): {python_pid}")
    print(f"PID via /proc/self/stat: {proc_pid}")
    if python_pid != proc_pid:
        print("As duas consultas retornaram PIDs diferentes.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
