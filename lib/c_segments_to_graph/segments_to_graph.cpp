#include "segments_to_graph.hpp"

int main(int argc, char **argv) {
  auto segments = read_segments();
  std::cerr.precision(10);
  std::cerr << "bin" << std::endl;
  for (auto s : segments) {
    std::cerr << s << std::endl;
  }
  CDT cdt(segments.begin(), segments.end());
//  CGAL::make_conforming_Delaunay_2(cdt);
  auto skeleton_edges = compute_skeleton_edges(cdt);

  std::cout.precision(10);
  for (auto edge : skeleton_edges) {
    std::cout << edge.p << " " << edge.q << " "
              << CGAL::squared_distance(edge.p, edge.q) << " " << edge.d
              << std::endl;
  }

  return 0;
}
