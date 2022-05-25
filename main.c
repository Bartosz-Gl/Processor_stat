#include <stdio.h>
#include "Threads.h"
#include <pthread.h>
#include <string.h>



int main() {
    pthread_t tid_reader, tid_analyzer, tid_printer, tid_watchdog, tid_logger;
    pthread_mutex_init(&lock_data, NULL);
    pthread_mutex_init(&lock_logger, NULL);

    struct data *data =(struct data*) malloc(sizeof(struct data));
    if (data == NULL) {
        printf("Error allocating memory\n");
        return -1;
    }
    if(initialize(data) == -1) {
        return -1;
    }


    pthread_create(&tid_reader, NULL, &reader, (void*)data);
    //printf("start analyzer\n");
    pthread_create(&tid_analyzer, NULL, &analyzer, (void*) data);
    //printf("start analyzer2\n");
    pthread_create(&tid_printer, NULL,  &printer, (void*) data);
    //printf("start analyzer3\n");
    pthread_create(&tid_watchdog, NULL,  &watchdog, (void*) data);
    //printf("start analyzer4\n");
    pthread_create(&tid_logger, NULL,  &logger, (void*) data);

    sleep(50);
    data->exit = 0;
    if(data->watchdog_flags->logger_flag || data->watchdog_flags->reader_flag || data->watchdog_flags->analyzer_flag || data->watchdog_flags->printer_flag) {
        printf("Error: at least 1 thread stopped working, program will stop.\nPlease wait for \"end\" message\n");
        sleep(1);
    }
    pthread_join(tid_reader, NULL);
    pthread_join(tid_analyzer, NULL);
    pthread_join(tid_printer, NULL);
    pthread_join(tid_watchdog, NULL);
    pthread_join(tid_logger, NULL);


    clear_data(data);
    printf("end\n");

    return 0;
}

