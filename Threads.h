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

#ifndef PROCESSOR_STAT_THREADS_H
#define PROCESSOR_STAT_THREADS_H

struct cpustat {
    char *core_number;
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
    char *message;
    int flag;
    char *path;
};

struct data {
    struct cpustat* stats_array;
    struct logger_data* logger_data;
    int number_of_procs;
    char* path;
    int test_flag;
    double* cpu_usage;
    int exit;
};

//reader
void reader(void* args);

//analyzer
void analyzer(void* args);

//printer
void printer(void* args);

//watchdog
void watchdog();

//logger
void logger(void* args);


double calculate_load(struct cpustat *prev, struct cpustat *cur);

int initialize(struct data* data);
void clear_data(struct data* data);
int read_data(FILE *fp, struct data* data, int offset );

#endif //PROCESSOR_STAT_THREADS_H
