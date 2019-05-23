#ifndef SEGMENTS_TO_GRAPH_HPP
#define SEGMENTS_TO_GRAPH_HPP

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_conformer_2.h>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Itag = CGAL::Exact_predicates_tag;
using CDT = CGAL::Constrained_Delaunay_triangulation_2<K, CGAL::Default, Itag>;
using FH = CDT::Face_handle;
using VH = CDT::Vertex_handle;

using Point = CDT::Point;
using Segment = K::Segment_2;
using Line = K::Line_2;

struct SkeletonEdge {
  Point p, q;
  double d;
};

std::vector<Segment> read_segments() {
  std::vector<Segment> segments;
  std::copy(std::istream_iterator<Segment>(std::cin),
            std::istream_iterator<Segment>(), std::back_inserter(segments));
  return segments;
}

FH find_inner_face(const CDT &cdt) {
  std::vector<FH> queue;
  std::unordered_set<FH> settled;
  queue.push_back(cdt.infinite_face());
  while (!queue.empty()) {
    auto fh = queue.back();
    queue.pop_back();
    if (settled.count(fh) > 0) // already visited ?
      continue;
    settled.insert(fh);
    for (int i = 0; i < 3; i++) {
      CDT::Edge e(fh, i);
      if (cdt.is_constrained(e)) {
        auto inner_face = e.first->neighbor(e.second);
        return inner_face;
      }
      CDT::Face_handle n = fh->neighbor(i);
      if (settled.count(n) > 0)
        continue;
      queue.push_back(n);
    }
  }
}

std::unordered_set<CDT::Face_handle> find_all_inner_faces(const CDT &cdt) {
  auto init_fh = find_inner_face(cdt);
  std::vector<CDT::Face_handle> queue;
  std::unordered_set<FH> settled;
  queue.push_back(init_fh);
  while (!queue.empty()) {
    auto fh = queue.back();
    queue.pop_back();
    if (settled.count(fh) > 0)
      continue;
    settled.insert(fh);
    for (int i = 0; i < 3; i++) {
      CDT::Edge e(fh, i);
      if (cdt.is_constrained(e)) {
        continue; // stay inside polygon
      }
      CDT::Face_handle n = fh->neighbor(i);
      if (settled.count(n) > 0)
        continue;
      queue.push_back(n);
    }
  }
  return settled;
}

std::vector<SkeletonEdge> compute_skeleton_edges(CDT &cdt) {
  CGAL::make_conforming_Delaunay_2(cdt);

  auto inner_faces = find_all_inner_faces(cdt);
  std::vector<SkeletonEdge> skeleton_edges;

  for (auto eit = cdt.finite_edges_begin(); eit != cdt.finite_edges_end();
       ++eit) {
    if (cdt.is_constrained(*eit))
      continue;
    auto e_1 = *eit;
    auto f_1 = e_1.first;
    if (cdt.is_infinite(f_1))
      continue;
    Point c_1;
    c_1 = cdt.circumcenter(f_1);
    auto r_1 = CGAL::squared_distance(c_1, f_1->vertex(0)->point());

    auto e_2 = cdt.mirror_edge(*eit);
    auto f_2 = e_2.first;
    if (cdt.is_infinite(f_2))
      continue;
    Point c_2;
    c_2 = cdt.circumcenter(f_2);
    auto r_2 = CGAL::squared_distance(c_2, f_2->vertex(0)->point());

    auto s = cdt.segment(*eit);
    auto l = Line(s);

    double clearing;
    if (l.oriented_side(c_1) == l.oriented_side(c_2)) {
      clearing = std::sqrt(std::min(r_1, r_2));
    } else {
      clearing = std::sqrt(s.squared_length()) / 2.;
    }

    if (c_1 == c_2)
      continue;

    CDT::Locate_type loc;
    int loci;
    auto fc1 = cdt.locate(c_1, loc, loci, f_1);
    if (cdt.is_infinite(fc1))
      continue;
    auto fc2 = cdt.locate(c_2, loc, loci, f_2);
    if (cdt.is_infinite(fc2))
      continue;

    auto face_circulator = cdt.line_walk(c_1, c_2, fc1);
    if (face_circulator.is_empty())
      continue;

    std::vector<FH> faces;
    faces.push_back(face_circulator.handle());
    while (cdt.triangle(face_circulator.handle()).has_on_unbounded_side(c_2)) {
      ++face_circulator;
      faces.push_back(face_circulator.handle());
    }
    if (std::none_of(faces.begin(), faces.end(), [&inner_faces](FH h) {
          return inner_faces.count(h) == 0;
        }))
      skeleton_edges.push_back({c_1, c_2, clearing});
  }

  return skeleton_edges;
}

#endif /* SEGMENTS_TO_GRAPH_HPP */
