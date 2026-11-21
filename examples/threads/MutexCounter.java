import java.util.Locale;

public final class MutexCounter {
    private static final int THREAD_COUNT = 4;
    private static final int INCREMENTS = 100_000;

    private static final class Counter {
        private long value;

        private synchronized void increment() {
            // O monitor do objeto protege todo o incremento.
            value++;
        }
    }

    private static void joinStarted(Thread[] workers, int started) {
        boolean interrupted = false;
        for (int i = 0; i < started; i++) {
            while (true) {
                try {
                    workers[i].join();
                    break;
                } catch (InterruptedException error) {
                    interrupted = true;
                }
            }
        }

        if (interrupted) {
            Thread.currentThread().interrupt();
        }
    }

    public static void main(String[] args) throws InterruptedException {
        Counter counter = new Counter();
        Thread[] workers = new Thread[THREAD_COUNT];
        for (int i = 0; i < workers.length; i++) {
            workers[i] = new Thread(() -> {
                for (int step = 0; step < INCREMENTS; step++) {
                    counter.increment();
                }
            });
        }

        int started = 0;
        try {
            for (; started < workers.length; started++) {
                workers[started].start();
            }
        } finally {
            // Aguarda também os trabalhadores iniciados antes de uma eventual falha.
            joinStarted(workers, started);
        }
        if (Thread.currentThread().isInterrupted()) {
            throw new InterruptedException("Execução interrompida enquanto aguardava as threads.");
        }

        long expected = (long) THREAD_COUNT * INCREMENTS;
        System.out.printf(Locale.ROOT, "Contador final: %d (esperado: %d)%n", counter.value, expected);
        if (counter.value != expected) {
            throw new IllegalStateException("O contador final difere do valor esperado.");
        }
    }
}
