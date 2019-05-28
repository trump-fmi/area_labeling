#include <CGAL/FPU.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>

#include "label_fit.hpp"

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point_2 = K::Point_2;
using Circle_2 = K::Circle_2;
using Polygon_2 = CGAL::Polygon_2<K>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<K>;

using namespace boost::python;

class LabelFitter {
public:
  void build(list points) {
    std::vector<Point_2> c_points;
    for (size_t i(0); i < len(points); ++i) {
      double x = extract<double>(points[i][0]);
      double y = extract<double>(points[i][1]);
      c_points.emplace_back(x, y);
    }

    poly = Polygon_with_holes_2(Polygon_2(c_points.begin(), c_points.end()));
  }

  tuple label(double aspect, double radius, tuple center) {
    double x = extract<double>(center[0]);
    double y = extract<double>(center[1]);
    Point_2 res = compute_labelling(poly, aspect, Circle_2{{x, y}, radius * radius});

    return make_tuple(res.x(), res.y());
  }
private:
  Polygon_with_holes_2 poly;
};

BOOST_PYTHON_MODULE(c_label_fit) {
  class_<LabelFitter>("LabelFitter")
      .def("build", &LabelFitter::build)
      .def("label", &LabelFitter::label);
}
