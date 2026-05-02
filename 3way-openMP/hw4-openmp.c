#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define BATCH_SIZE   4096
#define MAX_LINE_LEN 8192


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


int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: openmprun -np <N> %s <filename>\n", argv[0]);
        return 1;
    }

    int thread_count;

    if (argc >= 3) {
        thread_count = atoi(argv[2]);
    } else {
        thread_count = 4;
    }

    

    if (thread_count < 1) {
        fprintf(stderr, "Thread count must be at least 1\n");
        return 1;
    }


    omp_set_num_threads(thread_count);

    FILE *file = fopen(argv[1], "r");


    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char *buffer = malloc(BATCH_SIZE * MAX_LINE_LEN * sizeof(char));
    int *results = malloc(BATCH_SIZE * sizeof(int));

    if (buffer == NULL || results == NULL) {
        perror("Error allocating memory");
        free(buffer);
        free(results);
        fclose(file);
        return 1;
    }

    long long total_lines = 0;
    while (1) {

        int count = 0;

        while (count < BATCH_SIZE && fgets(buffer + count * MAX_LINE_LEN, MAX_LINE_LEN, file) != NULL) {

            int len = strlen(buffer + count * MAX_LINE_LEN);

            if (len > 0 && buffer[count * MAX_LINE_LEN + len - 1] == '\n') {
                buffer[count * MAX_LINE_LEN + len - 1] = '\0';
            }

            count++;

        }

        if (count == 0) {
            break;
        }

        #pragma omp parallel for

        for (int i = 0; i < count; i++) {
            results[i] = find_max_ascii(buffer + i * MAX_LINE_LEN);
        }

        for (int i = 0; i < count; i++) {
            printf("Line %lld: Max ASCII = %d\n",
                   total_lines + i + 1,
                   results[i]);
        }

        total_lines += count;
    }

    free(buffer);
    free(results);
    fclose(file);

    return 0;
}

