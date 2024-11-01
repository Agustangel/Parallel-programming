#include <chrono>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <bit>
#include <omp.h>
#include "matrix.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>

namespace po = boost::program_options;

static void fillSubmatrix(matrix& dest, const matrix& src,
                          std::size_t row_offset, std::size_t col_offset) {
  auto dest_rows = dest.getProxyRows();
  std::for_each(dest_rows.begin(), dest_rows.end(), [&](auto& dest_row) {
    auto src_row = src[row_offset++];
    std::copy(src_row.begin() + col_offset,
              src_row.begin() + col_offset + dest.ncols(), dest_row.begin());
  });
}

// I refused the idea of removing an explicit loop, because this will
// increase the number of nested loops from 1 to 4.
static void combineSubmatrix(matrix& dest, const matrix& src11,
                             const matrix& src12, const matrix& src21,
                             const matrix& src22) {
  std::size_t offset = src11.nrows();
  for (std::size_t i = 0; i < offset; ++i) {
    for (std::size_t j = 0; j < offset; ++j) {
      dest[i][j] = src11[i][j];
      dest[i][j + offset] = src12[i][j];
      dest[i + offset][j] = src21[i][j];
      dest[i + offset][j + offset] = src22[i][j];
    }
  }
}

static matrix algorithmStrassen(const matrix& A, const matrix& B) {
  assert(A.isSquare() && B.isSquare());
  assert(A.nrows() == B.nrows());
  assert(std::popcount(A.nrows()) == 1 && std::popcount(B.nrows()) == 1);

  std::size_t size = A.nrows();
  // Use the usual multiplication for a small matrix size.
  if (size <= 2)
    return A * B;

  std::size_t n = size >> 1;
  matrix A11{n, n}, A12{n, n}, A21{n, n}, A22{n, n};
  matrix B11{n, n}, B12{n, n}, B21{n, n}, B22{n, n};

  fillSubmatrix(A11, A, 0, 0);
  fillSubmatrix(A12, A, 0, n);
  fillSubmatrix(A21, A, n, 0);
  fillSubmatrix(A22, A, n, n);

  fillSubmatrix(B11, B, 0, 0);
  fillSubmatrix(B12, B, 0, n);
  fillSubmatrix(B21, B, n, 0);
  fillSubmatrix(B22, B, n, n);

  // Recursive part of the algorithm.
  matrix P1{}, P2{}, P3{}, P4{}, P5{}, P6{}, P7{};

  #pragma omp task shared(P1, P2, P3, P4, P5, P6, P7)
    P1 = algorithmStrassen(A11 + A22, B11 + B22);

  #pragma omp task shared(P1, P2, P3, P4, P5, P6, P7)
    P2 = algorithmStrassen(A21 + A22, B11);
  
  #pragma omp task shared(P1, P2, P3, P4, P5, P6, P7)
    P3 = algorithmStrassen(A11, B12 - B22);
  
  #pragma omp task shared(P1, P2, P3, P4, P5, P6, P7)
    P4 = algorithmStrassen(A22, B21 - B11);

  #pragma omp task shared(P1, P2, P3, P4, P5, P6, P7)
    P5 = algorithmStrassen(A11 + A12, B22);

  #pragma omp task shared(P1, P2, P3, P4, P5, P6, P7)
    P6 = algorithmStrassen(A21 - A11, B11 + B12);

  #pragma omp task shared(P1, P2, P3, P4, P5, P6, P7)
    P7 = algorithmStrassen(A12 - A22, B21 + B22);

  #pragma omp taskwait

  // Calculating the result submatrices.
  matrix C11 = P1 + P4 - P5 + P7;
  matrix C12 = P3 + P5;
  matrix C21 = P2 + P4;
  matrix C22 = P1 - P2 + P3 + P6;

  matrix C{size, size};
  combineSubmatrix(C, C11, C12, C21, C22);

  return C;
}

int main(int argc, char** argv) {
  std::size_t size = 8;

  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "size", po::value<std::size_t>(&size),
      "Size of the square matrix must be a power of two");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  matrix A = matrix::square_unit(size);
  matrix C {};

  auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel shared(A) 
{
  #pragma omp single nowait 
		C = algorithmStrassen(A, A);
}
  auto  finish = std::chrono::high_resolution_clock::now();

  auto elapsed = std::chrono::duration<double, std::milli>(finish - start);
  std::cout << "Calculation took " << elapsed.count() << "ms to run"
            << std::endl;

  return 0;
}
