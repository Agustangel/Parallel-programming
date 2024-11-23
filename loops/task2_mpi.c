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
        fprintf(stderr, "Запуск: %s <i_size> <j_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    unsigned i_size = strtoul(argv[1], NULL, 10);
    unsigned j_size = strtoul(argv[2], NULL, 10);
    if(i_size <= 0 || j_size <- 0){
        fprintf(stderr, "Недопустимое значение размера.\n");
        exit(EXIT_FAILURE);        
    }

    // Определение диапазона столбцов для текущего процесса
    int cols_per_proc = j_size / size;
    int extra_cols = j_size % size;
    int start_col = rank * cols_per_proc + (rank < extra_cols ? rank : extra_cols);
    int end_col = start_col + cols_per_proc + (rank < extra_cols ? 1 : 0);

    double *a = NULL, *local_array = NULL;
    int *send_counts = NULL, *displs = NULL;
    if (rank == 0) {
        a = get_array(i_size, j_size);
        // Определяем размеры и смещения для Scatterv
        send_counts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        int offset = 0;
        for (int i = 0; i < size; i++) {
            send_counts[i] = (cols_per_proc + (i < extra_cols ? 1 : 0)) * i_size;
            displs[i] = offset;
            offset += send_counts[i];
        }
    }

    // Локальный массив столбцов
    unsigned local_cols = end_col - start_col;
    local_array = (double*)malloc(i_size * local_cols * sizeof(double));

    // Распределяем столбцы двумерного массива a по процессам
    MPI_Scatterv(
        a, send_counts, displs, MPI_DOUBLE, local_array,
        i_size * local_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD
    );

    double start = MPI_Wtime();
    for (unsigned i = 3; i < i_size; i++) {
        for (unsigned j = 0; j < local_cols; j++) {
            local_array[i * j_size + j] = sin(3 * local_array[(i - 3) * j_size + (j + 2)]);
        }
        // Обмен на границе

    }

    // Собираем результат обратно на процессе 0
    MPI_Gatherv(
        local_array, i_size * local_cols, MPI_DOUBLE, a,
        send_counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD
    );
    MPI_Barrier(MPI_COMM_WORLD);

    double end = MPI_Wtime();
    if (rank == 0) {
        double elapsed_time = end - start;
        printf("Время выполнения: %.6f секунд\n", elapsed_time);
        write_file(a, i_size, j_size);
        free(a);
        free(send_counts);
        free(displs);
    }

    free(local_array);
    MPI_Finalize();

    return 0;
}
