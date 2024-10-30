#pragma once

#include <algorithm>
#include <concepts>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

class matrix {
  std::vector<int> buffer;
  std::size_t rows = 0;
  std::size_t cols = 0;

 public:
  matrix(std::size_t rows, std::size_t cols, int val = {})
      : buffer(rows * cols, val), rows{rows}, cols{cols} {}

  template <std::input_iterator Iter>
  matrix(std::size_t rows, std::size_t cols, Iter frst, Iter lst)
      : matrix{rows, cols} {
    std::size_t count = rows * cols;
    std::copy_if(frst, lst, buffer.begin(),
                 [&count](const auto&) { return count && count--; });
  }

  matrix(matrix&& rhs) noexcept
      : buffer(std::move(rhs.buffer)), rows(rhs.rows), cols(rhs.cols) {
    rhs.rows = 0;
    rhs.cols = 0;
  }

  matrix(const matrix& rhs)
      : buffer(rhs.buffer), rows(rhs.rows), cols(rhs.cols) {}

  static matrix square_unit(std::size_t size) { return matrix{size, size, 1}; }

 private:
  class proxy_row {
    int* row_ptr = nullptr;
    int* row_end_ptr = nullptr;

   public:
    proxy_row() = default;
    proxy_row(int* begin_ptr, std::size_t cols)
        : row_ptr{begin_ptr}, row_end_ptr{row_ptr + cols} {}

    int& operator[](std::size_t idx) { return row_ptr[idx]; }
    const int& operator[](std::size_t idx) const { return row_ptr[idx]; }

    auto begin() const { return row_ptr; }
    auto end() const { return row_end_ptr; }
  };

  class const_proxy_row {
    const int* row_ptr = nullptr;
    const int* row_end_ptr = nullptr;

   public:
    const_proxy_row() = default;
    const_proxy_row(const int* begin_ptr, std::size_t cols)
        : row_ptr{begin_ptr}, row_end_ptr{row_ptr + cols} {}

    const int& operator[](std::size_t idx) const { return row_ptr[idx]; }

    auto begin() const { return row_ptr; }
    auto end() const { return row_end_ptr; }
  };

 public:
  proxy_row operator[](unsigned idx) {
    return proxy_row{&*buffer.begin() + cols * idx, cols};
  }
  const_proxy_row operator[](unsigned idx) const {
    return const_proxy_row{&*buffer.begin() + cols * idx, cols};
  }

  std::vector<proxy_row> getProxyRows() {
    std::vector<proxy_row> rows(nrows());
    std::generate(rows.begin(), rows.end(),
                  [this, i = 0]() mutable { return (*this)[i++]; });
    return rows;
  }

  matrix& operator=(const matrix& rhs) noexcept {
    if (this == &rhs)
      return *this;
    rows = rhs.rows;
    cols = rhs.cols;
    buffer = rhs.buffer;

    return *this;
  }

  matrix& operator+=(const matrix& rhs) {
    if ((rows != rhs.rows) || (cols != rhs.cols))
      throw std::runtime_error("Unsuitable matrix sizes");

    matrix tmp{rows, cols, begin(), end()};
    std::transform(tmp.begin(), tmp.end(), rhs.begin(), tmp.begin(),
                   std::plus<>());
    *this = std::move(tmp);
    return *this;
  }

  matrix& operator-=(const matrix& rhs) {
    if ((rows != rhs.rows) || (cols != rhs.cols))
      throw std::runtime_error("Unsuitable matrix sizes");

    matrix tmp{rows, cols, begin(), end()};
    std::transform(tmp.begin(), tmp.end(), rhs.begin(), tmp.begin(),
                   std::minus<>());
    *this = std::move(tmp);
    return *this;
  }

  matrix& operator*=(const matrix& rhs) {
    if (cols != rhs.rows)
      throw std::runtime_error("Unsuitable matrix sizes");

    matrix res{rows, rhs.cols};
    matrix tmp{rhs};
    tmp.transpose();

    std::transform(res.begin(), res.end(), res.begin(), [&](int& elem) {
      auto i = (&elem - &res.buffer[0]) / res.ncols();
      auto j = (&elem - &res.buffer[0]) % res.ncols();
      elem = std::inner_product((*this)[i].begin(), (*this)[i].end(),
                                tmp[j].begin(), 0);
      return elem;
    });

    *this = std::move(res);
    return *this;
  }

  matrix& transpose() & {
    if (isSquare()) {
      std::for_each(begin(), end(), [this](int& elem) {
        auto i = (&elem - &buffer[0]) / cols;
        auto j = (&elem - &buffer[0]) % cols;
        if (i < j)
          std::swap((*this)[i][j], (*this)[j][i]);
      });
      return *this;
    }
    matrix transposed{cols, rows};
    std::for_each(begin(), end(), [&](int& elem) {
      auto i = (&elem - &buffer[0]) / cols;
      auto j = (&elem - &buffer[0]) % cols;
      transposed[j][i] = std::move(elem);
    });
    *this = std::move(transposed);
    return *this;
  }

  std::size_t nrows() const { return rows; }
  std::size_t ncols() const { return cols; }

  std::vector<int>::iterator begin() { return buffer.begin(); }
  std::vector<int>::iterator end() { return buffer.end(); }

  std::vector<int>::const_iterator begin() const { return buffer.cbegin(); }
  std::vector<int>::const_iterator end() const { return buffer.cend(); }

  bool isSquare() const { return nrows() == ncols(); }

  void dump(std::ostream& os) const {
    os << "n_rows = " << nrows() << std::endl;
    os << "n_cols = " << ncols() << std::endl;
    for (std::size_t i = 0; i < nrows(); ++i) {
      os << "| ";
      for (std::size_t j = 0; j < ncols(); ++j) {
        os << (*this)[i][j] << " ";
      }
      os << "|" << std::endl;
    }
  }
};

// clang-format off
matrix operator+(const matrix &lhs, const matrix &rhs) { auto res = lhs; res += rhs; return res; }
matrix operator-(const matrix &lhs, const matrix &rhs) { auto res = lhs; res -= rhs; return res; }
matrix operator*(const matrix &lhs, const matrix &rhs) { auto res = lhs; res *= rhs; return res; }
// clang-format on
