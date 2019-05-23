#include <iostream>
#include <string>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>

#include "longest_paths.hpp"

using namespace boost::python;

class LongestPaths {
public:
  void build(list edges) {
    std::vector<Segment> segments;
    for (size_t i(0); i < len(edges); ++i) {
      auto p = edges[i][0];
      auto q = edges[i][1];
      double weight = extract<double>(edges[i][2]);
      double clearing = extract<double>(edges[i][3]);
      double px = extract<double>(p[0]);
      double py = extract<double>(p[1]);
      double qx = extract<double>(q[0]);
      double qy = extract<double>(q[1]);
      segments.push_back({{px, py}, {qx, qy}, weight, clearing});
    }
    graph = from_edges(segments);
  }

  boost::python::list paths(double aspect, double STEP, size_t k = 10) {
    auto res = find_distinct_paths(graph, aspect, STEP, k);

    list py_res;
    for (auto &path : res) {
      list py_path; 
      for (Vertex v : path) {
        Point p = graph[v];
        py_path.append(boost::python::make_tuple(p.x, p.y));
      }
      py_res.append(py_path);
    }
    return py_res;
  }
private:
  Graph graph;
};

BOOST_PYTHON_MODULE(c_longest_paths) {
  class_<LongestPaths>("LongestPaths")
      .def("build", &LongestPaths::build)
      .def("paths", &LongestPaths::paths);
}
