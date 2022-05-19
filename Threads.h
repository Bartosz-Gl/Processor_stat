//
// Created by jojojej on 19.05.2022.
//
#include <pthread.h>

#ifndef PROCESSOR_STAT_THREADS_H
#define PROCESSOR_STAT_THREADS_H

struct cpustat {
    unsigned long t_user;
    unsigned long t_nice;
    unsigned long t_system;
    unsigned long t_idle;
    unsigned long t_iowait;
    unsigned long t_irq;
    unsigned long t_softirq;
    int flag;
};

//reader
void reader(char* path, struct cpustat* data);

//analyzer
void analyzer(struct cpustat* data);

//printer
void printer(struct cpustat* data);

//watchdog
void watchdog();

//logger

#endif //PROCESSOR_STAT_THREADS_H
