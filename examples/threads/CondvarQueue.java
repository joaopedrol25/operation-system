import java.util.Locale;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

public final class CondvarQueue {
    private static final int ITEM_COUNT = 5;

    private static final class SlotQueue {
        private final ReentrantLock lock = new ReentrantLock();
        private final Condition canRead = lock.newCondition();
        private final Condition canWrite = lock.newCondition();
        private Integer item;
        private boolean closed;

        private boolean send(int nextItem) throws InterruptedException {
            lock.lockInterruptibly();
            try {
                while (item != null && !closed) {
                    canWrite.await();
                }
                if (closed) {
                    return false;
                }

                item = nextItem;
                canRead.signal();
                return true;
            } finally {
                lock.unlock();
            }
        }

        private Integer receive() throws InterruptedException {
            lock.lockInterruptibly();
            try {
                // await libera o lock e o readquire antes de testar o predicado.
                while (item == null && !closed) {
                    canRead.await();
                }

                Integer result = item;
                item = null;
                canWrite.signal();
                return result;
            } finally {
                lock.unlock();
            }
        }

        private void close() {
            lock.lock();
            try {
                closed = true;
                canRead.signalAll();
                canWrite.signalAll();
            } finally {
                lock.unlock();
            }
        }
    }

    private static void produceItems(SlotQueue queue) {
        try {
            for (int item = 1; item <= ITEM_COUNT; item++) {
                if (!queue.send(item)) {
                    return;
                }
            }
        } catch (InterruptedException error) {
            Thread.currentThread().interrupt();
        } finally {
            queue.close();
        }
    }

    private static void joinProducer(Thread producer) {
        boolean interrupted = false;
        while (true) {
            try {
                producer.join();
                break;
            } catch (InterruptedException error) {
                interrupted = true;
            }
        }

        if (interrupted) {
            Thread.currentThread().interrupt();
        }
    }

    public static void main(String[] args) throws InterruptedException {
        SlotQueue queue = new SlotQueue();
        Thread producer = new Thread(() -> produceItems(queue));
        int consumed = 0;
        int total = 0;
        boolean inOrder = true;

        producer.start();
        try {
            // A thread principal é a consumidora.
            Integer item;
            while ((item = queue.receive()) != null) {
                consumed++;
                total += item;
                inOrder = inOrder && item == consumed;
            }
        } finally {
            // Se o consumidor for interrompido, libera o produtor antes do join.
            queue.close();
            joinProducer(producer);
        }
        if (Thread.currentThread().isInterrupted()) {
            throw new InterruptedException("Execução interrompida enquanto aguardava o produtor.");
        }

        int expected = ITEM_COUNT * (ITEM_COUNT + 1) / 2;
        System.out.printf(Locale.ROOT, "Itens consumidos: %d, soma: %d (esperada: %d)%n",
                consumed, total, expected);
        if (!inOrder || consumed != ITEM_COUNT || total != expected) {
            throw new IllegalStateException("Os itens consumidos diferem da sequência esperada.");
        }
    }
}
