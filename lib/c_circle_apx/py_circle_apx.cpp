#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/python.hpp>

#include "circle_apx.hpp"

using namespace boost::python;

std::vector<circle_apx_nsp::Point> to_c_points(list &points) {
  std::vector<circle_apx_nsp::Point> c_points;
  for (size_t i = 0; i < len(points); ++i) {
    auto py_point = points[i];
    auto x = py_point[0];
    auto y = py_point[1];
    c_points.push_back({extract<double>(x), extract<double>(y)});
  }
  return c_points;
}

tuple circle_apx(list points) {
  auto c_points = to_c_points(points);
  circle_apx_nsp::Circle c = apx_circle(c_points);

  return make_tuple(make_tuple(c.x, c.y), c.r);
}

tuple circle_apx_nl(list points) {
  auto c_points = to_c_points(points);
  circle_apx_nsp::Circle c = apx_circle_nl(c_points);
  return make_tuple(make_tuple(c.x, c.y), c.r);
}

BOOST_PYTHON_MODULE(c_circle_apx) { 
  def("circle_apx", &circle_apx); 
  def("circle_apx_nl", &circle_apx_nl); 
};
