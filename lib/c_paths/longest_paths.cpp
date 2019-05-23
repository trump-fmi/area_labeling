#include "longest_paths.hpp"
#include <iostream>


int main(int argc, char *argv[]) {
  std::vector<Segment> segs;
  double px, py, qx, qy, w, c;
  while (std::cin >> px >> py >> qx >> qy >> w >> c)
    segs.push_back({{px, py}, {qx, qy}, w, c});
  auto graph = from_edges(segs);

  double aspect = .1;
  auto paths = find_distinct_paths(graph, aspect, 2., 10);

  for (auto path : paths) {
    for (auto v : path) {
      std::cout << graph[v].x << graph[v].y << std::endl;
    }
    std::cout << std::endl;
  }

  return 0;
}
