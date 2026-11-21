#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ITEM_COUNT = 5 };

struct queue {
    pthread_mutex_t mutex;
    pthread_cond_t can_read;
    pthread_cond_t can_write;
    int item;
    bool full;
    bool closed;
};

static void check_pthread(int error, const char *operation)
{
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(error));
        exit(EXIT_FAILURE);
    }
}

static void queue_send(struct queue *queue, int item)
{
    check_pthread(pthread_mutex_lock(&queue->mutex), "pthread_mutex_lock");

    /* A espera libera o mutex e volta com ele adquirido para testar o estado. */
    while (queue->full) {
        check_pthread(pthread_cond_wait(&queue->can_write, &queue->mutex), "pthread_cond_wait (escrita)");
    }

    queue->item = item;
    queue->full = true;
    check_pthread(pthread_cond_signal(&queue->can_read), "pthread_cond_signal (leitura)");
    check_pthread(pthread_mutex_unlock(&queue->mutex), "pthread_mutex_unlock");
}

static void *produce_items(void *argument)
{
    struct queue *queue = argument;
    for (int number = 1; number <= ITEM_COUNT; ++number) {
        queue_send(queue, number);
    }

    check_pthread(pthread_mutex_lock(&queue->mutex), "pthread_mutex_lock");
    queue->closed = true;
    check_pthread(pthread_cond_signal(&queue->can_read), "pthread_cond_signal (leitura)");
    check_pthread(pthread_mutex_unlock(&queue->mutex), "pthread_mutex_unlock");
    return NULL;
}

static bool queue_receive(struct queue *queue, int *item)
{
    check_pthread(pthread_mutex_lock(&queue->mutex), "pthread_mutex_lock");

    /* Um despertar espúrio pode acontecer. O predicado é testado em laço. */
    while (!queue->full && !queue->closed) {
        check_pthread(pthread_cond_wait(&queue->can_read, &queue->mutex), "pthread_cond_wait (leitura)");
    }

    const bool found = queue->full;
    if (found) {
        *item = queue->item;
        queue->full = false;
        check_pthread(pthread_cond_signal(&queue->can_write), "pthread_cond_signal (escrita)");
    }

    check_pthread(pthread_mutex_unlock(&queue->mutex), "pthread_mutex_unlock");
    return found;
}

int main(void)
{
    struct queue queue = {0};
    pthread_t producer;
    int consumed = 0;
    int sum = 0;
    bool in_order = true;

    check_pthread(pthread_mutex_init(&queue.mutex, NULL), "pthread_mutex_init");
    check_pthread(pthread_cond_init(&queue.can_read, NULL), "pthread_cond_init (leitura)");
    check_pthread(pthread_cond_init(&queue.can_write, NULL), "pthread_cond_init (escrita)");

    const int create_error = pthread_create(&producer, NULL, produce_items, &queue);
    if (create_error == 0) {
        /* A thread principal consome somente se o produtor foi criado. */
        int item;
        while (queue_receive(&queue, &item)) {
            ++consumed;
            sum += item;
            if (item != consumed) {
                in_order = false;
            }
        }
        check_pthread(pthread_join(producer, NULL), "pthread_join");
    }

    check_pthread(pthread_cond_destroy(&queue.can_write), "pthread_cond_destroy (escrita)");
    check_pthread(pthread_cond_destroy(&queue.can_read), "pthread_cond_destroy (leitura)");
    check_pthread(pthread_mutex_destroy(&queue.mutex), "pthread_mutex_destroy");
    check_pthread(create_error, "pthread_create");

    const int expected = ITEM_COUNT * (ITEM_COUNT + 1) / 2;
    printf("Itens consumidos: %d, soma: %d (esperada: %d)\n", consumed, sum, expected);
    if (!in_order) {
        fprintf(stderr, "Os itens chegaram fora da ordem esperada.\n");
    }
    return in_order && consumed == ITEM_COUNT && sum == expected ? EXIT_SUCCESS : EXIT_FAILURE;
}
