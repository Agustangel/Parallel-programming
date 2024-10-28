#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

class matrix {
  std::vector<int> buffer;
  std::size_t rows = 0;
  std::size_t cols = 0;

public:
  matrix(std::size_t rows, std::size_t cols)
      : buffer(rows * cols, int{}), rows{rows}, cols{cols} {}

  template <typename Iter>
  matrix(std::size_t rows, std::size_t cols, Iter frst, Iter lst)
      : matrix{rows, cols} {
    std::size_t count = rows * cols;
    std::copy_if(frst, lst, buffer.begin(),
                 [&count](const auto &) { return count && count--; });
  }

  matrix(matrix &&rhs) noexcept
      : buffer(std::move(rhs.buffer)), rows(rhs.rows), cols(rhs.cols) {
    rhs.rows = 0;
    rhs.cols = 0;
  }

  matrix(const matrix &rhs)
      : buffer(rhs.buffer), rows(rhs.rows), cols(rhs.cols) {}

private:
  class proxy_row {
    int *row_ptr = nullptr;
    int *row_end_ptr = nullptr;

  public:
    proxy_row() = default;
    proxy_row(int *begin_ptr, std::size_t cols)
        : row_ptr{begin_ptr}, row_end_ptr{row_ptr + cols} {}

    int &operator[](std::size_t idx) { return row_ptr[idx]; }
    const int &operator[](std::size_t idx) const { return row_ptr[idx]; }
  };

  class const_proxy_row {
    const int *row_ptr = nullptr;
    const int *row_end_ptr = nullptr;

  public:
    const_proxy_row() = default;
    const_proxy_row(const int *begin_ptr, std::size_t cols)
        : row_ptr{begin_ptr}, row_end_ptr{row_ptr + cols} {}

    const int &operator[](std::size_t idx) const { return row_ptr[idx]; }
  };

public:
  proxy_row operator[](unsigned idx) {
    return proxy_row{&*buffer.begin() + ncols() * idx, ncols()};
  }
  const_proxy_row operator[](unsigned idx) const {
    return const_proxy_row{&*buffer.begin() + ncols() * idx, ncols()};
  }

  std::size_t nrows() const { return rows; }
  std::size_t ncols() const { return cols; }

  bool isSquare() const { return rows == cols; }
};
