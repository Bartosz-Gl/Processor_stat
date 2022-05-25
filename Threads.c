//
// Created by jojojej on 19.05.2022.
//
#include "Threads.h"

//reader
void reader(void* args){
    struct data* data = (struct data*) args;

    while(data->exit) {
        while (data->test_flag!=0) ;
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

        sleep(1);

        //printf("przeszo\n");
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
    struct data* data = (struct data*)args;
    data->cpu_usage = (double *) malloc(data->number_of_procs * sizeof(double));
    if(data->cpu_usage == NULL) {
        data->logger_data->message = (char *) malloc(100 * sizeof(char));
        data->logger_data->message = "malloc error";
        data->logger_data->flag = 1;
        return;
    }

    while(data->exit) {
        while (data->test_flag != 1) {
            sleep(1);
        }
        for (int i = 0; i < data->number_of_procs; i++) {
            data->cpu_usage[i] = calculate_load(data->stats_array + i, data->stats_array + i + data->number_of_procs);
        }
        data->test_flag = 2;
    }
}

//printer
void printer(void* args) {
    //print data->data_processed
    struct data *data = (struct data *) args;
    while (data->exit) {
        sleep(1);
        for (int i = 0; i < data->number_of_procs; i++) {
            printf("%s - %f\n", data->stats_array[i].core_number, data->cpu_usage[i]);
        }
        data->logger_data->message = (char *) malloc(100 * sizeof(char));
        data->logger_data->message = "printing";
        data->logger_data->flag = 1;
        data->test_flag = 0;
    }

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
    struct cpustat *cpu_stat_array = data->stats_array;
    do {
        data->stats_array->offset = offset+i;
        if ((*(cpu_stat_array + i + offset)).core_number == NULL) {
            (*(cpu_stat_array + i + offset)).core_number = (char *) malloc(10 * sizeof(char));
        }
        char *numbers = (char *) malloc(200 * sizeof(char));
        eerro = fscanf(fp, "%s ", (*(cpu_stat_array + i + offset)).core_number);
        int j = 0;
        do{
            numbers[j]= (char)fgetc(fp);
            j++;
        } while (numbers[j-1] != '\n');
        if (eerro != 1) {
            data->logger_data->message = "file read error";
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

        i++;
    } while (i < data->number_of_procs);
    return 0;
}
