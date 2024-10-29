#include "matrix.hpp"
#include <iostream>

int main() {
  std::vector<int> data_A = {1, 1, 1, 1, 1, 1};
  matrix A{3, 2, data_A.begin(), data_A.end()};

  std::vector<int> data_B = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  matrix B{3, 3, data_B.begin(), data_B.end()};

  B.dump(std::cout);
  B *= A;
  B.dump(std::cout);

  return 0;
}