#include "matrix.hpp"
#include <algorithm>
#include <iostream>

static void fillSubmatrix(matrix &dest, const matrix &src,
                          std::size_t row_offset, std::size_t col_offset) {
  auto dest_rows = dest.getProxyRows();
  std::for_each(dest_rows.begin(), dest_rows.end(), [&](auto &dest_row) {
    auto src_row = src[row_offset++];
    std::copy(src_row.begin() + col_offset,
              src_row.begin() + col_offset + dest.ncols(), dest_row.begin());
  });
}

// Strassen's algorithm requires square matrices of the same size.
// Strassen's algorithm requires the size of matrices of degree two.
static matrix algorithmStrassen(const matrix &A, const matrix &B,
                                std::size_t n) {
  // Use the usual multiplication for a small matrix size.
  if (n <= 64)
    return A * B;

  n = n >> 1;
  matrix A11(n, n), A12(n, n), A21(n, n), A22(n, n);
  matrix B11(n, n), B12(n, n), B21(n, n), B22(n, n);

  fillSubmatrix(A11, A, 0, 0);
  fillSubmatrix(A12, A, 0, n);
  fillSubmatrix(A21, A, n, 0);
  fillSubmatrix(A22, A, n, n);

  fillSubmatrix(B11, B, 0, 0);
  fillSubmatrix(B12, B, 0, n);
  fillSubmatrix(B21, B, n, 0);
  fillSubmatrix(B22, B, n, n);
  return A;
}

int main() {
  std::vector<int> data_A = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};
  matrix A{4, 4, data_A.begin(), data_A.end()};
  matrix B{2, 2};
  matrix C{2, 2};
  matrix D{2, 2};
  matrix E{2, 2};
  fillSubmatrix(B, A, 0, 0);
  fillSubmatrix(C, A, 0, 2);
  fillSubmatrix(D, A, 2, 0);
  fillSubmatrix(E, A, 2, 2);
  A.dump(std::cout);
  B.dump(std::cout);
  C.dump(std::cout);
  D.dump(std::cout);
  E.dump(std::cout);

  return 0;
}