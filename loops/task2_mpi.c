#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_file(double* a, unsigned i_size, unsigned j_size) {
  FILE* ff = fopen("result.txt", "w");
  for (unsigned i = 0; i < i_size; i++) {
    for (unsigned j = 0; j < j_size; j++) {
      fprintf(ff, "%f ", a[i * j_size + j]);
    }
    fprintf(ff, "\n");
  }
  fclose(ff);
}

double* get_array(unsigned rows, unsigned cols) {
  double* a = calloc(rows * cols, sizeof(double));
  if (!a) {
    perror("Ошибка выделения памяти для массива\n");
    exit(EXIT_FAILURE);
  }
  for (unsigned i = 0; i < rows; i++) {
    for (unsigned j = 0; j < cols; j++) {
      a[i * cols + j] = 10 * i + j;  // Инициализация
    }
  }
  return a;
}

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  MPI_Status status;

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc < 3) {
    fprintf(stderr, "Запуск: %s <i_size> <j_size>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  unsigned i_size = strtoul(argv[1], NULL, 10);
  unsigned j_size = strtoul(argv[2], NULL, 10);
  if (i_size <= 0 || j_size < -0) {
    fprintf(stderr, "Недопустимое значение размера.\n");
    exit(EXIT_FAILURE);
  }

  double* a = get_array(i_size, j_size);

  // Определение диапазона столбцов для текущего процесса
  int cols_per_proc = j_size / size;
  int extra_cols = j_size % size;
  int start_col =
      rank * cols_per_proc + (rank < extra_cols ? rank : extra_cols);
  int end_col = start_col + cols_per_proc + (rank < extra_cols ? 1 : 0);
  int local_cols = end_col - start_col;

  int *recv_counts = NULL, *displs = NULL;
  double* gathered_a = NULL;
  if (rank == 0) {
    recv_counts = malloc(size * sizeof(int));
    displs = malloc(size * sizeof(int));
    gathered_a = malloc(i_size * j_size * sizeof(double));
    int offset = 0;
    for (int i = 0; i < size; i++) {
      recv_counts[i] = local_cols * i_size;
      displs[i] = offset;
      offset += recv_counts[i];
    }
  }

  double start = MPI_Wtime();
  for (unsigned i = 3; i < i_size; i++) {
    for (unsigned j = start_col; j < end_col; j++) {
      a[i * j_size + j] = sin(3 * a[(i - 3) * j_size + (j + 2)]);
    }
    // Обмен на границе
    // 3 -> 2 | 1 -> 0
    // 2 -> 1 |
    if (rank % 2 == 0) {
      if (rank != size - 1)
        MPI_Recv(&a[i * j_size + end_col], 2, MPI_DOUBLE, rank + 1, 0,
                 MPI_COMM_WORLD, &status);
      if (rank != 0)
        MPI_Send(&a[i * j_size + start_col], 2, MPI_DOUBLE, rank - 1, 0,
                 MPI_COMM_WORLD);
    } else {
      MPI_Send(&a[i * j_size + start_col], 2, MPI_DOUBLE, rank - 1, 0,
               MPI_COMM_WORLD);
      if (rank != size - 1)
        MPI_Recv(&a[i * j_size + end_col], 2, MPI_DOUBLE, rank + 1, 0,
                 MPI_COMM_WORLD, &status);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // Локальный массив отправляем
  double* sendbuf = malloc(local_cols * i_size * sizeof(double));
  for (unsigned i = 0; i < i_size; i++) {
    memcpy(&sendbuf[i * local_cols], &a[i * j_size + start_col],
           local_cols * sizeof(double));
  }

  MPI_Gatherv(sendbuf, local_cols * i_size, MPI_DOUBLE, gathered_a, recv_counts,
              displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  double end = MPI_Wtime();
  if (rank == 0) {
    double elapsed_time = end - start;
    printf("Время выполнения: %.6f секунд\n", elapsed_time);
    write_file(gathered_a, i_size, j_size);
    free(gathered_a);
    free(recv_counts);
    free(displs);
  }

  free(sendbuf);
  free(a);
  MPI_Finalize();

  return 0;
}
