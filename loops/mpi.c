#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>


void write_file(double* a, unsigned i_size, unsigned j_size) {
    FILE *ff = fopen("result.txt", "w");
    for (unsigned i = 0; i < i_size; i++) {
        for (unsigned j = 0; j < j_size; j++) {
            fprintf(ff, "%f ", a[i * j_size + j]);
        }
        fprintf(ff, "\n");
    }
    fclose(ff);
}

double* get_array(unsigned rows, unsigned cols) {
    double *a = calloc(rows * cols, sizeof(double));
    if (!a) {
        perror("Ошибка выделения памяти для массива\n");
        exit(EXIT_FAILURE);
    }
    for (unsigned i = 0; i < rows; i++) {
        for (unsigned j = 0; j < cols; j++) {
            a[i * cols + j] = 10 * i + j; // Инициализация
        }
    }
    return a;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(argc < 3){
        if (rank == 0)
            fprintf(stderr, "Запуск: %s <i_size> <j_size>\n", argv[0]);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    unsigned i_size = strtoul(argv[1], NULL, 10);
    unsigned j_size = strtoul(argv[2], NULL, 10);
    if(i_size <= 0 || j_size <- 0){
        if (rank == 0)
            fprintf(stderr, "Недопустимое значение размера.\n");
        MPI_Finalize();
        exit(EXIT_FAILURE);        
    }

    // Определение диапазона строк для текущего процесса
    unsigned rows_per_proc = i_size / size;
    unsigned extra_rows = i_size % size;
    unsigned start_row = rank * rows_per_proc + (rank < extra_rows ? rank : extra_rows);
    unsigned end_row = start_row + rows_per_proc + (rank < extra_rows ? 1 : 0);

    // Локальный массив строк
    unsigned local_rows = end_row - start_row;
    double* local_array = get_array(local_rows, j_size);

    double start = MPI_Wtime();
    for (unsigned i = 0; i < local_rows; i++) {
        for (unsigned j = 0; j < j_size; j++) {
            local_array[i * j_size + j] = sin(2 * local_array[i * j_size + j]);
        }
    }

    // Сбор результатов в процессе 0
    double* global_array = NULL;
    if (rank == 0) {
        global_array = get_array(i_size, j_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(
        local_array, local_rows * j_size, MPI_DOUBLE,
        global_array, local_rows * j_size, MPI_DOUBLE,
        0, MPI_COMM_WORLD
    );
    MPI_Barrier(MPI_COMM_WORLD);

    double end = MPI_Wtime();
    if (rank == 0) {
        double elapsed_time = end - start;
        printf("Время выполнения: %.6f секунд\n", elapsed_time);
        write_file(ff, global_array, i_size, j_size);
        free(global_array);
    }

    free(local_array);
    MPI_Finalize();

    return 0;
}
