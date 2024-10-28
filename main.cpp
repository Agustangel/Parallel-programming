#include "matrix.hpp"

int main() {
  matrix A{5, 4};

  std::vector<int> data = {1, 2, 3, 4};
  matrix B{2, 2, data.begin(), data.end()};

  return 0;
}