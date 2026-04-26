#include <mpi.h>
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

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);



    if (argc < 2) {
        if (rank == 0)
            fprintf(stderr, "Usage: mpirun -np <N> %s <filename>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }


    FILE *file = NULL;
    if (rank == 0) {
        file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("Error opening file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    char *buffer = malloc(BATCH_SIZE * MAX_LINE_LEN * sizeof(char));
    int *results = malloc(BATCH_SIZE * sizeof(int));
    int *batch_counts = malloc(size * sizeof(int));
    int *offsets = malloc(size * sizeof(int));
    int *receive_counts = malloc(size * sizeof(int));
    int *receive_offsets = malloc(size * sizeof(int));

    if (buffer == NULL || results == NULL || batch_counts == NULL || offsets == NULL || receive_counts == NULL || receive_offsets == NULL) {
        perror("Error allocating memory");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int max_lines_per_process = (BATCH_SIZE + size - 1) / size;
    char *line_buffer = malloc(max_lines_per_process * MAX_LINE_LEN * sizeof(char));
    int *line_results = malloc(max_lines_per_process * sizeof(int));
    if (line_buffer == NULL || line_results == NULL) {
        perror("Error allocating memory for line buffer or results");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    

    long long total_lines = 0;
    while (1) {

        int count = 0;

        if (rank == 0) {
            while (count < BATCH_SIZE && fgets(buffer + count * MAX_LINE_LEN, MAX_LINE_LEN, file) != NULL) {
                int len = strlen(buffer + count * MAX_LINE_LEN);
                if (len > 0 && buffer[count * MAX_LINE_LEN + len - 1] == '\n') {
                    buffer[count * MAX_LINE_LEN + len - 1] = '\0';
                }
                count++;
            }
        }

        MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (count == 0) {
            break;
        }

        int base = count / size;
        int remainder = count % size;
        int offset = 0;
        for (int i = 0; i < size; i++) {
            int lines_for_i = base + (i < remainder ? 1 : 0); // change
            batch_counts[i] = lines_for_i * MAX_LINE_LEN; // change
            offsets[i] = offset;
            receive_counts[i]  = lines_for_i;
            receive_offsets[i] = offset / MAX_LINE_LEN;
            offset += batch_counts[i];
            
        }

        MPI_Scatterv(buffer, batch_counts, offsets, MPI_CHAR, line_buffer, max_lines_per_process * MAX_LINE_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);

        for (int i = 0; i < batch_counts[rank] / MAX_LINE_LEN; i++) {
            line_results[i] = find_max_ascii(line_buffer + i * MAX_LINE_LEN);
        }

        int *send_counts = malloc(size * sizeof(int));
        int *send_offsets = malloc(size * sizeof(int));
        int send_offset = 0;
        for (int i = 0; i < size; i++) {
            send_counts[i] = batch_counts[i] / MAX_LINE_LEN;
            send_offsets[i] = send_offset;
            send_offset += send_counts[i];
        }

        MPI_Gatherv(line_results, batch_counts[rank] / MAX_LINE_LEN, MPI_INT, results, receive_counts, receive_offsets, MPI_INT, 0, MPI_COMM_WORLD);

        free(send_counts);
        free(send_offsets);

        if (rank == 0) {
            for (int i = 0; i < count; i++) {
                printf("%lld: %d\n", total_lines + i, results[i]);
            }
            total_lines += count;
        }



    }

    
    if (rank == 0) {
        fclose(file);
    }


    free(buffer);
    free(results);
    free(batch_counts);
    free(offsets);
    free(receive_counts);
    free(receive_offsets);
    free(line_buffer);
    free(line_results);
    MPI_Finalize();
    return 0;

}
