#include <stdio.h>
#include "Threads.h"
#include <pthread.h>
#include <string.h>



int main() {
    pthread_t tid_reader, tid_analyzer, tid_printer, tid_watchdog, tid_logger;

    struct data *data =(struct data*) malloc(sizeof(struct data));
    if(initialize(data) == -1) {
        return -1;
    }

    pthread_create(&tid_reader, NULL, (void *(*)(void *)) &reader, (void*)data);
    printf("start analyzer\n");
    pthread_create(&tid_analyzer, NULL, (void *(*)(void *)) &analyzer, (void*) data);
    printf("start analyzer2\n");
    pthread_create(&tid_printer, NULL, (void *(*)(void *)) &printer, (void*) data);
    printf("start analyzer3\n");
    pthread_create(&tid_watchdog, NULL, (void *(*)(void *)) &watchdog, (void*) data);
    printf("start analyzer4\n");
    pthread_create(&tid_logger, NULL, (void *(*)(void *)) &logger, (void*) data);

    if(getchar() == 'q') {
        data->exit = 0;
    }

    pthread_join(tid_reader, NULL);
    pthread_join(tid_analyzer, NULL);
    pthread_join(tid_printer, NULL);
    pthread_join(tid_watchdog, NULL);
    pthread_join(tid_logger, NULL);

    clear_data(data);

    return 0;
}

