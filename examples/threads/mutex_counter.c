#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { THREAD_COUNT = 4, INCREMENTS = 100000 };

struct counter {
    pthread_mutex_t mutex;
    unsigned long value;
};

static void check_pthread(int error, const char *operation)
{
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(error));
        exit(EXIT_FAILURE);
    }
}

static void *increment_counter(void *argument)
{
    struct counter *counter = argument;

    for (int i = 0; i < INCREMENTS; ++i) {
        check_pthread(pthread_mutex_lock(&counter->mutex), "pthread_mutex_lock");
        /* O mutex protege a leitura e a escrita que formam o incremento. */
        ++counter->value;
        check_pthread(pthread_mutex_unlock(&counter->mutex), "pthread_mutex_unlock");
    }

    return NULL;
}

int main(void)
{
    struct counter counter = {.value = 0};
    pthread_t threads[THREAD_COUNT];
    size_t started = 0;
    int create_error = 0;

    check_pthread(pthread_mutex_init(&counter.mutex, NULL), "pthread_mutex_init");

    for (; started < THREAD_COUNT; ++started) {
        create_error = pthread_create(&threads[started], NULL, increment_counter, &counter);
        if (create_error != 0) {
            break;
        }
    }

    for (size_t i = 0; i < started; ++i) {
        check_pthread(pthread_join(threads[i], NULL), "pthread_join");
    }
    check_pthread(pthread_mutex_destroy(&counter.mutex), "pthread_mutex_destroy");
    check_pthread(create_error, "pthread_create");

    const unsigned long expected = (unsigned long)THREAD_COUNT * INCREMENTS;
    printf("Contador final: %lu (esperado: %lu)\n", counter.value, expected);
    return counter.value == expected ? EXIT_SUCCESS : EXIT_FAILURE;
}
