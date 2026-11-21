#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { THREAD_COUNT = 2 };

struct job {
    int number;
    int square;
};

static void *calculate_square(void *argument)
{
    struct job *job = argument;
    job->square = job->number * job->number;
    return NULL;
}

int main(void)
{
    pthread_t threads[THREAD_COUNT];
    struct job jobs[THREAD_COUNT] = {{.number = 6}, {.number = 7}};
    size_t started = 0;
    int create_error = 0;

    /* Cada thread recebe um elemento próprio que existe até o fim de main. */
    for (; started < THREAD_COUNT; ++started) {
        create_error = pthread_create(&threads[started], NULL, calculate_square, &jobs[started]);
        if (create_error != 0) {
            break;
        }
    }

    /* Aguarda inclusive as threads iniciadas antes de uma eventual falha. */
    for (size_t i = 0; i < started; ++i) {
        int error = pthread_join(threads[i], NULL);
        if (error != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }

    if (create_error != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(create_error));
        return EXIT_FAILURE;
    }

    /* A ordem da saída é independente da ordem de execução das threads. */
    for (size_t i = 0; i < THREAD_COUNT; ++i) {
        printf("Thread %zu: %d x %d = %d\n", i + 1, jobs[i].number, jobs[i].number, jobs[i].square);
    }

    return EXIT_SUCCESS;
}
