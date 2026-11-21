"""Equivalente didático de condvar_queue.c usando threading.Condition do Python."""

import sys
from threading import Condition, Thread


ITEM_COUNT = 5


class SlotQueue:
    def __init__(self) -> None:
        self._changed = Condition()
        self._item: int | None = None
        self._closed = False

    def send(self, item: int) -> bool:
        with self._changed:
            while self._item is not None and not self._closed:
                self._changed.wait()
            if self._closed:
                return False

            self._item = item
            self._changed.notify_all()
            return True

    def receive(self) -> int | None:
        with self._changed:
            # wait libera o lock e o readquire antes de testar o predicado.
            while self._item is None and not self._closed:
                self._changed.wait()

            item = self._item
            self._item = None
            self._changed.notify_all()
            return item

    def close(self) -> None:
        with self._changed:
            self._closed = True
            self._changed.notify_all()


def produce_items(queue: SlotQueue) -> None:
    try:
        for item in range(1, ITEM_COUNT + 1):
            if not queue.send(item):
                return
    finally:
        queue.close()


def main() -> int:
    queue = SlotQueue()
    producer = Thread(target=produce_items, args=(queue,))
    started = False
    consumed = 0
    total = 0
    in_order = True

    try:
        try:
            producer.start()
        except RuntimeError as error:
            print(f"Falha ao iniciar a thread produtora: {error}", file=sys.stderr)
            return 1
        started = True

        # A thread principal consome apenas após o início do produtor.
        while (item := queue.receive()) is not None:
            consumed += 1
            total += item
            in_order = in_order and item == consumed
    finally:
        # Libera o produtor mesmo se o consumidor for interrompido.
        queue.close()
        if started:
            producer.join()

    expected = ITEM_COUNT * (ITEM_COUNT + 1) // 2
    print(f"Itens consumidos: {consumed}, soma: {total} (esperada: {expected})")
    if not in_order:
        print("Os itens chegaram fora da ordem esperada.", file=sys.stderr)
    return 0 if in_order and consumed == ITEM_COUNT and total == expected else 1


if __name__ == "__main__":
    raise SystemExit(main())
