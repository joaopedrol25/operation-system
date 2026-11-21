import java.util.Locale;

public final class ThreadJoin {
    private static final class Job {
        private final int number;
        private int square;

        private Job(int number) {
            this.number = number;
        }

        private void calculateSquare() {
            square = number * number;
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
        Job[] jobs = {new Job(6), new Job(7)};
        Thread[] workers = new Thread[jobs.length];
        for (int i = 0; i < jobs.length; i++) {
            workers[i] = new Thread(jobs[i]::calculateSquare);
        }

        int started = 0;
        try {
            for (; started < workers.length; started++) {
                workers[started].start();
            }
        } finally {
            // Aguarda todas as threads iniciadas mesmo em caso de interrupção.
            joinStarted(workers, started);
        }
        if (Thread.currentThread().isInterrupted()) {
            throw new InterruptedException("Execução interrompida enquanto aguardava as threads.");
        }

        for (int i = 0; i < jobs.length; i++) {
            Job job = jobs[i];
            System.out.printf(Locale.ROOT, "Thread %d: %d x %d = %d%n",
                    i + 1, job.number, job.number, job.square);
        }
    }
}
