"""Equivalente didático de pthread_join.c usando threading.Thread do Python."""

from dataclasses import dataclass
import sys
from threading import Thread


@dataclass
class Job:
    number: int
    square: int = 0


def calculate_square(job: Job) -> None:
    job.square = job.number * job.number


def main() -> int:
    # Cada thread recebe seu próprio objeto de trabalho.
    jobs = [Job(6), Job(7)]
    workers = [Thread(target=calculate_square, args=(job,)) for job in jobs]
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

    # A ordem de exibição é definida pelo programa principal após os joins.
    for index, job in enumerate(jobs, start=1):
        print(f"Thread {index}: {job.number} x {job.number} = {job.square}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
