//
// Created by jojojej on 19.05.2022.
//
#include "Threads.h"



//reader
void *reader(void* args){
    struct data* data = (struct data*) args;

    while(data->exit) {
        while (data->test_flag!=0) ;
        if(data->watchdog_flags->reader_flag)
            data->watchdog_flags->reader_flag=false;
        FILE *fp = fopen(data->path, "r");
        if (fp == NULL) {
            pthread_mutex_lock(&lock_logger);
            memcpy(data->logger_data->message, "Error opening file", strlen("Error opening file"));
            data->logger_data->flag = 1;
            return 0;
        }
        int skip = fgetc(fp);

        while (skip != '\n') {
            skip = fgetc(fp);
        }
        pthread_mutex_lock(&lock_data);
        if(read_data(fp,data,0)!=0){
            pthread_mutex_lock(&lock_logger);
            memcpy(data->logger_data->message, "Error reading file", strlen("Error reading file"));
            data->logger_data->flag = 1;
            pthread_mutex_unlock(&lock_data);
            return 0;
        }
        pthread_mutex_unlock(&lock_data);

        sleep(1);

        fclose(fp);

        FILE *fp2 = fopen("/proc/stat", "r");
        if (fp2 == NULL) {
            pthread_mutex_lock(&lock_logger);
            memcpy(data->logger_data->message, "Error opening file", strlen("Error opening file"));
            data->logger_data->flag = 1;
            return 0;
        }
        int skip2 = fgetc(fp2);
        while (skip2 != '\n') {
            skip2 = fgetc(fp2);
        }
        pthread_mutex_lock(&lock_data);
        if(read_data(fp2,data, data->number_of_procs)!=0){
            pthread_mutex_lock(&lock_logger);
            memcpy(data->logger_data->message, "Error reading file", strlen("Error reading file"));
            data->logger_data->flag = 1;
            pthread_mutex_unlock(&lock_data);
            return 0;
        }
        pthread_mutex_unlock(&lock_data);
        data->test_flag = 1;
        fclose(fp2);
    }
    return 0;
}

//analyzer
void *analyzer(void* args){
    struct data* data = (struct data*)args;
    data->cpu_usage = (double *) malloc(data->number_of_procs * sizeof(double));
    if(data->cpu_usage == NULL) {
        pthread_mutex_lock(&lock_logger);
        memcpy(data->logger_data->message, "Error allocating memory", strlen("Error allocating memory"));
        data->logger_data->flag = 1;
        return 0;
    }

    while(data->exit) {
        while (data->test_flag != 1);
        if(data->watchdog_flags->analyzer_flag)
            data->watchdog_flags->analyzer_flag=false;
        pthread_mutex_lock(&lock_data);
        for (int i = 0; i < data->number_of_procs; i++) {
            data->cpu_usage[i] = calculate_load(data->stats_array + i, data->stats_array + i + data->number_of_procs);
        }
        pthread_mutex_unlock(&lock_data);
        data->test_flag = 2;
    }
    return 0;
}

//printer
void *printer(void* args) {
    //print data->data_processed
    struct data *data = (struct data *) args;
    while (data->exit) {
        if(data->watchdog_flags->printer_flag)
            data->watchdog_flags->printer_flag=false;
        sleep(1);
        pthread_mutex_lock(&lock_data);
        for (int i = 0; i < data->number_of_procs; i++) {
            printf("Usage of %s - %f\n", data->stats_array[i].core_number, data->cpu_usage[i]);
        }
        pthread_mutex_unlock(&lock_data);
        pthread_mutex_lock(&lock_logger);
        memcpy(data->logger_data->message, "Data printed\n", strlen("Data printed\n"));
        data->logger_data->flag = 1;
        data->test_flag = 0;
    }
    return 0;
}

//watchdog
void *watchdog(void* args) {
    struct data *data = (struct data *) args;
    sleep(1);
    while (data->exit) {
        sleep(1);
        data->watchdog_flags->analyzer_flag = true;
        data->watchdog_flags->printer_flag = true;
        data->watchdog_flags->reader_flag = true;
        data->watchdog_flags->logger_flag = true;
        sleep(2);
        if(data->watchdog_flags->logger_flag && data->exit ){
            printf("Logger is dead\n");
            pthread_mutex_unlock(&lock_logger);
            data->exit = 0;
            return 0;
        }
        if( data->watchdog_flags->reader_flag && data->exit) {
            pthread_mutex_lock(&lock_logger);
            printf("Reader is dead\n");
            memcpy(data->logger_data->message, "Reader is dead\n", strlen("Reader is dead\n"));
            data->logger_data->flag = 1;
            data->exit = 0;
            return 0;
        }
        if( data->watchdog_flags->analyzer_flag && data->exit) {
            pthread_mutex_lock(&lock_logger);
            printf("Analyzer is dead\n");
            memcpy(data->logger_data->message, "Analyzer is dead\n", strlen("Analyzer is dead\n"));
            data->logger_data->flag = 1;
            data->exit = 0;
            return 0;
        }
        if( data->watchdog_flags->printer_flag && data->exit) {
            pthread_mutex_lock(&lock_logger);
            printf("Printer is dead\n");
            memcpy(data->logger_data->message, "Printer is dead\n", strlen("Printer is dead\n"));
            data->logger_data->flag = 1;
            data->exit = 0;
            return 0;
        }

    }

    return 0;
}

void *logger(void *args){
    struct data* data = (struct data*)args;
    FILE *fp = fopen(data->logger_data->path, "a");
    if (fp == NULL) {
        printf("file open error for logger\n");
        return 0;
    }
    while(data->exit ) {
        while(data->logger_data->flag != 1){
            sleep(1);
            if(data->watchdog_flags->logger_flag)
                data->watchdog_flags->logger_flag=false;
        }
        fprintf(fp, "%s\n", data->logger_data->message);
        //clear message
        int message_lenght = (int)strlen(data->logger_data->message);
        for (int i = 0; i < message_lenght; i++) {
            data->logger_data->message[i] = '\0';
        }
        data->logger_data->flag = 0;
        pthread_mutex_unlock(&lock_logger);
    }
    fclose(fp);
    return 0;
}

double calculate_load(struct cpustat *prev, struct cpustat *cur)
{
    unsigned long idle_prev = (prev->t_idle) + (prev->t_iowait);
    unsigned long idle_cur = (cur->t_idle) + (cur->t_iowait);

    unsigned long nidle_prev = (prev->t_user) + (prev->t_nice) + (prev->t_system) + (prev->t_irq) + (prev->t_softirq);
    unsigned long nidle_cur = (cur->t_user) + (cur->t_nice) + (cur->t_system) + (cur->t_irq) + (cur->t_softirq);

    unsigned long total_prev = idle_prev + nidle_prev;
    unsigned long total_cur = idle_cur + nidle_cur;

    double totald = (double) total_cur - (double) total_prev;
    double idled = (double) idle_cur - (double) idle_prev;

    double cpu_perc = (1000 * (totald - idled) / totald + 1) / 10;

    return cpu_perc;
}

int initialize(struct data* data){

    if(data == NULL) {
        printf("malloc error\n");
        return -1;
    }
    int processors = get_nprocs();

    data->number_of_procs = processors;
    struct cpustat* cpu_stat_array;
    cpu_stat_array = (struct cpustat*)malloc(processors * 2 * sizeof(struct cpustat));
    if(cpu_stat_array == NULL) {
        printf("malloc error\n");
        return -1;
    }
    data->stats_array = cpu_stat_array;
    memcpy(data->path, "/proc/stat", strlen("/proc/stat"));
    data->test_flag = 0;
    data->exit = 1;
    data->logger_data->flag = 0;
    memcpy(data->logger_data->path, "/home/jojojej/Desktop/log.txt", strlen("/home/jojojej/Desktop/log.txt"));
    memcpy(data->logger_data->message, "Logger initialized", strlen("Logger initialized"));
    data->logger_data->flag = 1;
    return 0;
}

void clear_data(struct data* data){

    free(data->cpu_usage);
    free(data->stats_array);
    free(data);
}

int read_data(FILE *fp, struct data* data, int offset ) {
    int eerro;
    int i = 0;
    struct cpustat *cpu_stat_array = data->stats_array;
    do {
        data->stats_array->offset = offset+i;
        char *numbers = (char *) malloc(200 * sizeof(char));
        eerro = fscanf(fp, "%s ", (*(cpu_stat_array + i + offset)).core_number);
        int j = 0;
        do{
            numbers[j]= (char)fgetc(fp);
            j++;
        } while (numbers[j-1] != '\n');
        if (eerro != 1) {
            while(data->logger_data->flag );
            memcpy(data->logger_data->message, "Error reading data", strlen("Error reading data"));
            data->logger_data->flag = 1;
            return 1;
        }
        char *ptr;
        ((*(cpu_stat_array + i + offset)).t_user) = strtol((const char *) numbers, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_nice) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_system) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_idle) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_iowait) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_irq) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_softirq) = strtol(ptr, &ptr, 10);
        free(numbers);
        i++;
    } while (i < data->number_of_procs);
    return 0;
}
