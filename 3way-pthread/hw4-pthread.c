#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>


#define BATCH_SIZE   4096  
#define MAX_LINE_LEN 8192   

typedef struct {

    char **lines;   
    int *results;    
    int batch_count;   

    int num_threads;
    int workers_done;  
    int shutdown;  


    pthread_mutex_t mutex;
    pthread_cond_t  cond_go;   
    pthread_cond_t  cond_done;  
    int generation; 
} thread_data_t;

typedef struct {
    thread_data_t *shared_data;
    int thread_id;
} worker_arg_t;




int find_max_ascii(const char *str)
{
    int max = 0;
    if (str == NULL) {
        return 0; 
    }
    for (int i = 0; str[i] != '\0'; i++) {

        if (str[i] == '\n') {
            break; 
        }

        unsigned char c = (unsigned char)str[i];

        if (c > max) {
            max = c;
        }
    }
    return max;
}


static void *worker_thread(void *arg) {
    worker_arg_t *worker_arg = (worker_arg_t *)arg;
    thread_data_t *shared_data = worker_arg->shared_data;
    int thread_id = worker_arg->thread_id;
    int last_generation = 0;

    while(1) {

        pthread_mutex_lock(&shared_data->mutex);
        while(shared_data->generation == last_generation && !shared_data->shutdown) {
            pthread_cond_wait(&shared_data->cond_go, &shared_data->mutex);
        }

        if (shared_data->shutdown) {
            pthread_mutex_unlock(&shared_data->mutex);
            break; 
        }

        int generation = shared_data->generation;
        int count = shared_data->batch_count;
        int thread_amount = shared_data->num_threads;
        pthread_mutex_unlock(&shared_data->mutex);


        last_generation = generation;

        int chunk_size = (count + thread_amount - 1) / thread_amount;
        int start = thread_id * chunk_size;
        int end = start + chunk_size;
        if (end > count) {
            end = count;
        }

        for (int i = start; i < end; i++) {
            shared_data->results[i] = find_max_ascii(shared_data->lines[i]);
        }

        pthread_mutex_lock(&shared_data->mutex);
        shared_data->workers_done++;
        if (shared_data->workers_done == thread_amount) {
            pthread_cond_signal(&shared_data->cond_done);
        }
        pthread_mutex_unlock(&shared_data->mutex);
    }
    return NULL;
}







int main(int argc, char *argv[]) {



    if  (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int thread_amount;
    if (argc >= 3) {
        thread_amount = atoi(argv[2]);
    } else {
        thread_amount = 4; 
    }

    if (thread_amount <= 0) {
        thread_amount = 1;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r"); // use open instead probably 
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }


    thread_data_t shared_data;
    memset(&shared_data, 0, sizeof(shared_data));

    shared_data.num_threads = thread_amount;
    shared_data.lines = malloc(BATCH_SIZE * sizeof(char *));
    shared_data.results = malloc(BATCH_SIZE * sizeof(int));
    if (shared_data.lines == NULL || shared_data.results == NULL) {
        perror("malloc");
        fclose(file);
        return 1;
    }

    for (int i = 0; i < BATCH_SIZE; i++) {
        shared_data.lines[i] = malloc(MAX_LINE_LEN * sizeof(char));
        if (shared_data.lines[i] == NULL) {
            perror("malloc");
            fclose(file);
            return 1;
        }
    }



    pthread_mutex_init(&shared_data.mutex, NULL);
    pthread_cond_init(&shared_data.cond_go, NULL);
    pthread_cond_init(&shared_data.cond_done, NULL);

    pthread_t *threads = malloc(thread_amount * sizeof(pthread_t));
    worker_arg_t *worker_args = malloc(thread_amount * sizeof(worker_arg_t));

    if (threads == NULL || worker_args == NULL) {
        perror("Error allocating memory for threads or worker arguments");
        fclose(file);
        return 1;
    }

    for (int i = 0; i < thread_amount; i++) {
        worker_args[i].shared_data = &shared_data;
        worker_args[i].thread_id = i;
        if (pthread_create(&threads[i], NULL, worker_thread, &worker_args[i]) != 0) {
            perror("Error creating thread");
            fclose(file);
            return 1;
        }
    }

    long total_lines = 0;
    while (1) {
        int count = 0;
        while (count < BATCH_SIZE) {
            if (fgets(shared_data.lines[count], MAX_LINE_LEN, file) == NULL) {
                break; 
            }
            
            int len = strlen(shared_data.lines[count]);
            if (len > 0 && shared_data.lines[count][len - 1] == '\n') {
                shared_data.lines[count][len - 1] = '\0';
            }

            count++;
        }
        if (count == 0) {
            break;
        }

        pthread_mutex_lock(&shared_data.mutex);
        shared_data.batch_count = count;
        shared_data.workers_done = 0;
        shared_data.generation++;
        pthread_cond_broadcast(&shared_data.cond_go);
        pthread_mutex_unlock(&shared_data.mutex);

        while (shared_data.workers_done < thread_amount) {
            pthread_cond_wait(&shared_data.cond_done, &shared_data.mutex);
        }
        pthread_mutex_unlock(&shared_data.mutex);

        for(int i = 0; i < count; i++) {
            printf("Line %ld: Max ASCII = %d\n", total_lines + i + 1, shared_data.results[i]);
        }
        total_lines += count;

    }



    pthread_mutex_lock(&shared_data.mutex);
    shared_data.shutdown = 1;
    pthread_cond_broadcast(&shared_data.cond_go);
    pthread_mutex_unlock(&shared_data.mutex);

    for (int i = 0; i < thread_amount; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < BATCH_SIZE; i++) {
        free(shared_data.lines[i]);
    }
    free(shared_data.lines);
    free(shared_data.results);
    free(threads);
    free(worker_args);

    pthread_mutex_destroy(&shared_data.mutex);
    pthread_cond_destroy(&shared_data.cond_go);
    pthread_cond_destroy(&shared_data.cond_done);

    fclose(file);
    return 0;



}