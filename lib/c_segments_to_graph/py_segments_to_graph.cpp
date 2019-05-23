#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <cmath>

#include <boost/python.hpp>

#include "segments_to_graph.hpp"

using namespace boost::python;

std::vector<Segment> to_c_segments(list &segments) {
  std::vector<Segment> c_segments;
  for (size_t i = 0; i < len(segments); ++i) {
    auto py_seg = segments[i]; 
    auto p = py_seg[0];
    auto q = py_seg[1];
    c_segments.emplace_back(Point(
            extract<double>(p[0]), extract<double>(p[1])),
          Point(
            extract<double>(q[0]), extract<double>(q[1])));
  }
  return c_segments;
}

list to_graph(list segments) {
  auto c_segments = to_c_segments(segments);
  CDT cdt(c_segments.begin(), c_segments.end());
  auto skeleton_edges = compute_skeleton_edges(cdt);

  boost::python::list result;
  for (auto e : skeleton_edges) {
    result.append(make_tuple(
      e.p.x(), e.p.y(),
      e.q.x(), e.q.y(),
      std::sqrt(CGAL::squared_distance(e.p, e.q)),
      e.d
        )); 
  }


  return result;
}

BOOST_PYTHON_MODULE(c_segments_to_graph) {
  def("to_graph", &to_graph);
};
