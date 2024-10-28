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

  std::size_t getRows() const { return rows; }
  std::size_t getCols() const { return cols; }

  bool isSquare() const { return rows == cols; }
};