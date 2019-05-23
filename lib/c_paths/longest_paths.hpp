#ifndef LONGEST_PATHS_HPP
#define LONGEST_PATHS_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/labeled_graph.hpp>

#include <boost/graph/connected_components.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <vector>
#include <iostream>

using namespace boost;

struct Point {
  double x, y;
  bool operator==(const Point q) const { return x == q.x && y == q.y; }
  friend std::size_t hash_value(Point const &p) {
    std::size_t seed = 0;
    boost::hash_combine(seed, p.x);
    boost::hash_combine(seed, p.y);
    return seed;
  }
};

struct Segment {
  Point src, trg;
  double weight, cap;
};

using Node = Point;
using Graph = adjacency_list<
    vecS, vecS, undirectedS, Node,
    property<edge_weight_t, double, property<edge_capacity_t, double>>>;
using Vertex = Graph::vertex_descriptor;
using Edge = Graph::edge_descriptor;

using LGraph = labeled_graph<Graph, Point, hash_mapS>;

template <class Type> using Predicate = std::function<bool(Type)>;
using FGraph = filtered_graph<Graph, Predicate<Vertex>, Predicate<Edge>>;

template <class Edges> Graph from_edges(const Edges &edges) {
  LGraph lg;
  for (auto e : edges) {
    lg.add_vertex(e.src, e.src);
    lg.add_vertex(e.trg, e.trg);
    add_edge_by_label(e.src, e.trg, {e.weight, e.cap}, lg);
  }
  return lg.graph();
}

// returns a copy of the graph, only keeping edges with cap >= min_cap
Graph filter_capacity(Graph &g, double min_cap) {
  auto capacity_map = get(edge_capacity, g);
  Predicate<Edge> edges_with_cap = [min_cap, &capacity_map](const Edge e) {
    return get(capacity_map, e) >= min_cap;
  };
  auto g_cap_filtered = make_filtered_graph(g, edges_with_cap, keep_all());
  Graph g_cap;
  copy_graph(g_cap_filtered, g_cap);
  return g_cap;
}

// returns a copy of the graph, only keeping nodes with degree > 0
Graph filter_singleton_nodes(Graph &g) {
  Predicate<Vertex> nodes_with_edges = [&g](Vertex v) {
    return degree(v, g) > 0;
  };
  auto g_cap_filtered = make_filtered_graph(g, keep_all(), nodes_with_edges);
  Graph g_cap;
  copy_graph(g_cap_filtered, g_cap);
  return g_cap;
}

// returns a graph for every connected component of g
std::vector<Graph> into_components(Graph &g) {
  std::vector<size_t> components(num_vertices(g));
  size_t num_components =
      connected_components(g, boost::make_iterator_property_map(
                                  components.begin(), get(vertex_index, g)));

  std::vector<Graph> component_graphs;
  for (size_t i = 0; i < num_components; ++i) {
    Predicate<Vertex> nodes_of_comp = [i, &components](Vertex v) {
      return components[v] == i;
    };
    auto g_ci_filtered = make_filtered_graph(g, keep_all(), nodes_of_comp);
    Graph g_ci;
    copy_graph(g_ci_filtered, g_ci);
    component_graphs.push_back(g_ci);
  }
  return component_graphs;
}

std::vector<Vertex> component_vertices(Graph &g) {
  std::vector<size_t> components(num_vertices(g));
  size_t num_components =
      connected_components(g, boost::make_iterator_property_map(
                                  components.begin(), get(vertex_index, g)));
  std::vector<Vertex> representatives(num_components);
  for (auto vertex : make_iterator_range(vertices(g))) {
    representatives[components[vertex]] = vertex;
  }
  return representatives;
}

struct LastExaminedVisitor : public default_dijkstra_visitor {
  LastExaminedVisitor(Vertex *vd) : vd(vd) {}
  void examine_vertex(Vertex v, const Graph &) { *vd = v; }
  Vertex *vd;
};

template <class VertexIt>
Vertex furthest_node(const Graph &g, VertexIt vertices_begin,
                     VertexIt vertices_end) {
  std::vector<Vertex> pred(num_vertices(g));
  std::vector<double> dist(num_vertices(g));
  Vertex last_visited;
  dijkstra_shortest_paths(
      g, vertices_begin, vertices_end,
      make_iterator_property_map(pred.begin(), get(vertex_index, g)), // pred
      make_iterator_property_map(dist.begin(), get(vertex_index, g)), // dist
      get(edge_weight, g),                                            // weight
      get(vertex_index, g),                                           //
      std::less<double>(), closed_plus<double>(), // operations
      std::numeric_limits<double>::max(), 0.,     // operations
      LastExaminedVisitor(&last_visited));        // visitor
  return last_visited;
}

struct LastRootedExaminedVisitor : public default_dijkstra_visitor {
  LastRootedExaminedVisitor(std::vector<Vertex> &cf, std::vector<size_t> &root,
                            std::vector<Vertex> &pred)
      : component_furthest(cf), root(root), pred(pred) {}
  void examine_vertex(Vertex v, const Graph &) {
    auto v_pred = pred[v];
    auto v_root = root[v_pred];
    root[v] = v_root;
    component_furthest[v_root] = v;
  }
  std::vector<Vertex> &component_furthest;
  std::vector<size_t> &root;
  std::vector<Vertex> &pred;
};

template <class VertexIt>
std::vector<Vertex> furthest_rooted_nodes(const Graph &g,
                                          VertexIt vertices_begin,
                                          VertexIt vertices_end) {
  std::vector<Vertex> pred(num_vertices(g));
  std::vector<double> dist(num_vertices(g));
  std::vector<Vertex> component_furthest(vertices_begin, vertices_end);
  std::vector<size_t> root(num_vertices(g));
  for (auto i = 0, vit = vertices_begin; vit != vertices_end; ++vit, ++i) {
    root[*vit] = i;
  }

  dijkstra_shortest_paths(
      g, vertices_begin, vertices_end,
      make_iterator_property_map(pred.begin(), get(vertex_index, g)), // pred
      make_iterator_property_map(dist.begin(), get(vertex_index, g)), // dist
      get(edge_weight, g),                                            // weight
      get(vertex_index, g),                                           //
      std::less<double>(), closed_plus<double>(),                 // operations
      std::numeric_limits<double>::max(), 0.,                     // operations
      LastRootedExaminedVisitor(component_furthest, root, pred)); // visitor
  return component_furthest;
}

// Vertex furthest_node(const Graph &g, Vertex v) {
//  if (degree(v, g) == 0)
//    return v;
//  return furthest_node(g, &v, &v + 1);
//}

std::unordered_set<Vertex> component_furthest_vertices(Graph &g) {
  auto vertices = component_vertices(g);
  //  std::unordered_set<Vertex> furthest_vertices;
  //  std::transform(vertices.begin(), vertices.end(),
  //                 std::inserter(furthest_vertices, furthest_vertices.end()),
  //                 [&g](Vertex v) { return furthest_node(g, v); });
  auto furthest_vertices =
      furthest_rooted_nodes(g, vertices.begin(), vertices.end());
  return std::unordered_set<Vertex>(furthest_vertices.begin(),
                                    furthest_vertices.end());
}

std::vector<Vertex> unpack_path(const std::vector<Vertex> &pred, Vertex v) {
  std::vector<Vertex> path;
  path.push_back(v);
  while (pred[v] != v) {
    v = pred[v];
    path.push_back(v);
  }
  return path;
}

template <class G, class VertexIt>
std::pair<double, std::vector<Vertex>>
longest_path_from(const G &g, VertexIt vertices_begin, VertexIt vertices_end) {
  std::vector<typename G::vertex_descriptor> pred(num_vertices(g));
  std::vector<double> dist(num_vertices(g));
  typename G::vertex_descriptor last_visited;
  dijkstra_shortest_paths(
      g, vertices_begin, vertices_end,
      make_iterator_property_map(pred.begin(), get(vertex_index, g)), // pred
      make_iterator_property_map(dist.begin(), get(vertex_index, g)), // dist
      get(edge_weight, g),                                            // weight
      get(vertex_index, g),                                           //
      std::less<double>(), closed_plus<double>(), // operations
      std::numeric_limits<double>::max(), 0.,     // operations
      LastExaminedVisitor(&last_visited));        // visitor
  auto path = unpack_path(pred, last_visited);
  auto path_dist = dist[last_visited];
  return {path_dist, path};
}

std::vector<std::vector<Vertex>>
find_distinct_paths(Graph &graph, double aspect, double STEP, size_t k = 10) {
  std::vector<std::vector<Vertex>> paths;
  if (num_edges(graph) == 0)
    return paths;

  double mincap = get(edge_capacity, graph, *edges(graph).first);
  double maxcap = get(edge_capacity, graph, *edges(graph).first);
  for (auto e : make_iterator_range(edges(graph))) {
    double cap = get(edge_capacity, graph, e);
    mincap = std::min(mincap, cap);
    maxcap = std::max(maxcap, cap);
  }

  for (double CAP = maxcap;
       (CAP >= (mincap / STEP) || paths.size()==0) && paths.size() < k; CAP /= STEP) {

    auto cap_graph = filter_capacity(graph, CAP);
    auto node_set = component_furthest_vertices(cap_graph);
    while (paths.size() < k) {
      auto [dist1, path1] =
          longest_path_from(cap_graph, node_set.begin(), node_set.end());
      auto [dist, path] = 
          longest_path_from(cap_graph, path1.begin(), path1.begin() + 1);

      if (dist <= CAP / aspect)
        break;

      node_set.insert(path.begin(), path.end());
      paths.push_back(path);
    };
  }

  return paths;
}

#endif /* LONGEST_PATHS_HPP */
