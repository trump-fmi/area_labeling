#include <iostream>

#include "circle_apx.hpp"

int main(int argc, char *argv[]) {
  std::vector<circle_apx_nsp::Point> points = {{0, 2}, {1, 2}, {1.5, 1.5}, {2, 1}, {2, 0}};

  circle_apx_nsp::Circle c = apx_circle(points);

  std::cout << c.x << std::endl;
  std::cout << c.y << std::endl;
  std::cout << c.r << std::endl;

  return 0;
}
