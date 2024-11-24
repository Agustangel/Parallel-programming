#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void write_file(double** a, unsigned i_size, unsigned j_size) {
  FILE* ff = fopen("result.txt", "w");
  for (unsigned i = 0; i < i_size; i++) {
    for (unsigned j = 0; j < j_size; j++) {
      fprintf(ff, "%f ", a[i][j]);
    }
    fprintf(ff, "\n");
  }
  fclose(ff);
}

void free_array(double** a, unsigned i_size) {
  // Освобождение каждой строки
  for (unsigned i = 0; i < i_size; i++) {
    free(a[i]);
  }
  // Освобождение массива указателей
  free(a);
}

double** get_array(unsigned i_size, unsigned j_size) {
  // Выделение памяти для строк массима
  double** a = calloc(i_size, sizeof(double*));
  if (!a) {
    perror("Ошибка выделения памяти для строк массива\n");
    exit(EXIT_FAILURE);
  }
  // Выделение памяти для каждого ряда
  for (unsigned i = 0; i < i_size; i++) {
    a[i] = calloc(j_size, sizeof(double));
    if (!a[i]) {
      perror("Ошибка выделения памяти для столбцов массива\n");
      for (unsigned k = 0; k < i; k++) {
        free(a[k]);
      }
      free(a);
      exit(EXIT_FAILURE);
    }
  }
  //подготовительная часть – заполнение некими данными
  for (unsigned i = 0; i < i_size; i++) {
    for (unsigned j = 0; j < j_size; j++) {
      a[i][j] = 10 * i + j;
    }
  }
  return a;
}

int main(int argc, char** argv) {
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

  double** a = get_array(i_size, j_size);

  double start = omp_get_wtime();
  for (unsigned i = 0; i < i_size - 3; i++) {
#pragma omp parallel for schedule(static)
    for (unsigned j = 4; j < j_size; j++) {
      a[i][j] = sin(0.04 * a[i + 3][j - 4]);
    }
  }
  double end = omp_get_wtime();
  double elapsed_time = end - start;
  printf("Время выполнения: %.6f секунд\n", elapsed_time);

  write_file(a, i_size, j_size);

  free_array(a, i_size);

  return 0;
}
