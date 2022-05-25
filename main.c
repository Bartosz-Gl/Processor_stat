#include <stdio.h>
#include "Threads.h"
#include <pthread.h>
#include <string.h>


volatile sig_atomic_t exit_flag = 1;

void terminate(){
    exit_flag = 0;
}

int main() {
    pthread_t tid_reader, tid_analyzer, tid_printer, tid_watchdog, tid_logger;
    pthread_mutex_init(&lock_data, NULL);
    pthread_mutex_init(&lock_logger, NULL);

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = &terminate;
    sigaction(SIGTERM, &sa, NULL);

    struct data *data =(struct data*) malloc(sizeof(struct data));
    if (data == NULL) {
        printf("Error allocating memory\n");
        return -1;
    }
    if(initialize(data) == -1) {
        return -1;
    }


    pthread_create(&tid_reader, NULL, &reader, (void*)data);
    pthread_create(&tid_analyzer, NULL, &analyzer, (void*) data);
    pthread_create(&tid_printer, NULL,  &printer, (void*) data);
    pthread_create(&tid_watchdog, NULL,  &watchdog, (void*) data);
    pthread_create(&tid_logger, NULL,  &logger, (void*) data);

    while (exit_flag) {
        sleep(1);
    }
    data->exit = exit_flag;
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

