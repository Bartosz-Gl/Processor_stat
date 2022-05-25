//
// Created by jojojej on 19.05.2022.
//
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#ifndef PROCESSOR_STAT_THREADS_H
#define PROCESSOR_STAT_THREADS_H

pthread_mutex_t lock_data, lock_logger;

struct watchdog_flags{
    bool reader_flag;
    bool analyzer_flag;
    bool printer_flag;
    bool logger_flag;

};

struct cpustat {
    char core_number[10];
    unsigned long t_user;
    unsigned long t_nice;
    unsigned long t_system;
    unsigned long t_idle;
    unsigned long t_iowait;
    unsigned long t_irq;
    unsigned long t_softirq;
    int offset;
};

struct logger_data {
    char message[100];
    int flag;
    char path[35];
};

struct data {
    struct cpustat* stats_array;
    struct logger_data logger_data[1];
    struct watchdog_flags watchdog_flags[1];
    int number_of_procs;
    char path[15];
    int test_flag;
    double* cpu_usage;
    int exit;
};

//reader
void *reader(void* args);

//analyzer
void *analyzer(void* args);

//printer
void *printer(void* args);

//watchdog
void *watchdog(void *args);

//logger
void *logger(void* args);


double calculate_load(struct cpustat *prev, struct cpustat *cur);

int initialize(struct data* data);
void clear_data(struct data* data);
int read_data(FILE *fp, struct data* data, int offset );

#endif //PROCESSOR_STAT_THREADS_H
