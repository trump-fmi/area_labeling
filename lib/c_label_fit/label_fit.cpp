#include "label_fit.hpp"

int main() {
  double aspect;
  Point_2 center;
  double radius;

  std::cin >> aspect;
  std::cin >> radius;
  std::cin >> center;
  K::Circle_2 circle{center, radius * radius};

  Polygon_with_holes_2 ph;
  std::cin >> ph;

  std::cout << compute_labelling(ph, aspect, circle) << std::endl;

  return 0;
}
