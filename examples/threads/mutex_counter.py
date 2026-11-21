"""Equivalente didático de mutex_counter.c usando threading.Lock do Python."""

import sys
from threading import Lock, Thread


THREAD_COUNT = 4
INCREMENTS = 100_000


class Counter:
    def __init__(self) -> None:
        self.lock = Lock()
        self.value = 0

    def increment(self) -> None:
        for _ in range(INCREMENTS):
            # A leitura, a soma e a escrita formam uma única região crítica.
            with self.lock:
                self.value += 1


def main() -> int:
    counter = Counter()
    workers = [Thread(target=counter.increment) for _ in range(THREAD_COUNT)]
    started: list[Thread] = []

    try:
        for worker in workers:
            worker.start()
            started.append(worker)
    except RuntimeError as error:
        print(f"Falha ao iniciar uma thread: {error}", file=sys.stderr)
        return 1
    finally:
        for worker in started:
            worker.join()

    expected = THREAD_COUNT * INCREMENTS
    print(f"Contador final: {counter.value} (esperado: {expected})")
    return 0 if counter.value == expected else 1


if __name__ == "__main__":
    raise SystemExit(main())
