//
// Created by jojojej on 19.05.2022.
//
#include "Threads.h"

//reader
void reader(void* args){
    struct data* data = (struct data*) args;
    struct cpustat *cpu_stat_array = data->stats_array;
    while(data->exit) {
        FILE *fp = fopen(data->path, "r");
        if (fp == NULL) {
            data->logger_data->message = "file open error";
            data->logger_data->flag = 1;
            return;
        }
        int skip = fgetc(fp);

        while (skip != '\n') {
            skip = fgetc(fp);
        }
        if(read_data(fp,data,0)!=0){
            data->logger_data->message = "read error";
            data->logger_data->flag = 1;
            return;
        }

        sleep(2);
        fclose(fp);

        FILE *fp2 = fopen("/proc/stat", "r");
        if (fp2 == NULL) {
            data->logger_data->message = "file open error";
            data->logger_data->flag = 1;
            return;
        }
        int skip2 = fgetc(fp2);
        while (skip2 != '\n') {
            skip2 = fgetc(fp2);
        }

        if(read_data(fp2,data, data->number_of_procs)!=0){
            data->logger_data->message = "read error";
            data->logger_data->flag = 1;
            return;
        }



        data->test_flag = 1;
        fclose(fp2);
    }
}

//analyzer
void analyzer(void* args){

}

//printer
void printer(void* args){

}

//watchdog
void watchdog(){


}


void logger(void *args){
    struct data* data = (struct data*)args;
    FILE *fp = fopen(data->logger_data->path, "a");
    while(data->exit) {
        while(data->logger_data->flag != 1) {
            sleep(1);
        }
        if (fp == NULL) {
            printf("file open error for logger\n");
            return;
        }
        fprintf(fp, "%s\n", data->logger_data->message);
        data->logger_data->flag = 0;
    }
    fclose(fp);

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
    data->path = (char*)malloc(sizeof(char) * strlen("/proc/stat"));
    if(data->path == NULL) {
        printf("malloc error\n");
        return -1;
    }
    data->path = "/proc/stat";
    data->test_flag = 0;
    data->exit = 1;
    data->logger_data = (struct logger_data*)malloc(sizeof(struct logger_data));
    if(data->logger_data == NULL) {
        printf("malloc error\n");
        return -1;
    }
    data->logger_data->flag = 0;
    data->logger_data->path = (char*)malloc(sizeof(char) * strlen("/home/jojojej/Desktop/log.txt"));
    if(data->logger_data->path == NULL) {
        printf("malloc error\n");
        return -1;
    }
    data->logger_data->message = (char *) malloc(100 * sizeof(char));
    if(data->logger_data->message == NULL) {
        printf("malloc error\n");
        return -1;
    }
    data->logger_data->path = "/home/jojojej/Desktop/log.txt";
    return 0;
}

void clear_data(struct data* data){
    for (int i = 0; i < data->number_of_procs; i++) {
        free(data->stats_array[i].core_number);
    }
    free(data->stats_array->core_number);
    free(data->logger_data->message);
    free(data->stats_array);
    free(data->path);
    free(data->cpu_usage);
    free(data->logger_data->path);
    free(data->logger_data);
    free(data);
}

int read_data(FILE *fp, struct data* data, int offset ) {
    int eerro;
    int i = 0;
    int test;
    struct cpustat *cpu_stat_array = data->stats_array;
    do {
        if ((*(cpu_stat_array + i + offset)).core_number == NULL) {
            (*(cpu_stat_array + i + offset)).core_number = (char *) malloc(10 * sizeof(char));
        }
        char *numbers = (char *) malloc(200 * sizeof(char));
        eerro = fscanf(fp, "%s %s\n", (*(cpu_stat_array + i + offset)).core_number, numbers);
        if (eerro != 2) {
            data->logger_data->message = "file read error";
            data->logger_data->flag = 1;
            return 1;
        }
        char *ptr;
        ((*(cpu_stat_array + i + offset)).t_user) = strtol(numbers, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_nice) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_system) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_idle) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_iowait) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_irq) = strtol(ptr, &ptr, 10);
        ((*(cpu_stat_array + i + offset)).t_softirq) = strtol(ptr, &ptr, 10);

        test = fgetc(fp);
        while (test != '\n') {
            test = fgetc(fp);
        }
        i++;
    } while (i < data->number_of_procs);
    return 0;
}