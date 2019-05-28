#ifndef LABEL_FIT_HPP
#define LABEL_FIT_HPP

#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
#include <vector>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>

#include <boost/format.hpp>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point_2 = K::Point_2;
using Segment_2 = K::Segment_2;
using Polygon_2 = CGAL::Polygon_2<K>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<K>;
using Circle_2 = K::Circle_2;

struct Interval {
  K::FT begin, end;
  Interval() = default;
  Interval(K::FT b, K::FT e) {
    auto [smaller, larger] = std::minmax(b, e);
    begin = smaller;
    end = larger;
  }
  std::string str() {
    return (boost::format("[%1%, %2%]") % begin % end).str();
  }
  auto sub(Interval other) {
    std::pair<std::optional<Interval>, std::optional<Interval>> result;
    if (begin < std::min(end, other.begin))
      result.first = Interval(begin, std::min(end, other.begin));
    if (std::max(other.end, begin) < end)
      result.second = Interval(std::max(other.end, begin), end);
    return result;
  }
  auto len() { return end - begin; }
  auto mid() { return (end + begin) / 2.; }
};

struct AngleRange {
  K::FT begin, end;
  AngleRange(K::FT b, K::FT e) {
    if (e < b)
      std::swap(b, e);
    if (e - b > M_PI) {
      e -= 2 * M_PI;
      std::swap(b, e);
    }
    begin = b;
    end = e;
  }
  std::string str() {
    return (boost::format("[%1%, %2%]") % begin % end).str();
  }
};

auto point_angle(const Point_2 &p, const Point_2 &q) {
  return std::atan2(q.y() - p.y(), q.x() - p.x());
}

auto angle_range(const Point_2 &p, const Segment_2 &s) {
  return AngleRange{point_angle(p, s.source()), point_angle(p, s.target())};
}

auto distance(const Circle_2 &circle, const Segment_2 &segment) {
  auto c = circle.center();
  auto cs = CGAL::squared_distance(c, segment);
  auto cp = CGAL::squared_distance(c, segment.source());
  auto cq = CGAL::squared_distance(c, segment.target());
  if (cs > circle.squared_radius())
    return std::sqrt(cs) - std::sqrt(circle.squared_radius());
  if (circle.squared_radius() > std::max(cp, cq))
    return std::sqrt(circle.squared_radius()) - std::sqrt(std::max(cp, cq));
  return 0.;
}

struct Cup {
  K::FT height;
  Interval range;
  Cup(const Cup &cup) = default;
  Cup(const Segment_2 &s, const Circle_2 &c, double aspect) {
    double r = std::sqrt(c.squared_radius());
    double x = aspect * M_PI;
    double h_max = r * x / (1 + x);
    double h = std::min(h_max, distance(c, s));
    double rb = r - h;
    double H = 2 * h;
    double L = H / aspect;
    double delta_angle = L / rb / 2;
    auto angle_interval = angle_range(c.center(), s);
    range = Interval(angle_interval.begin - delta_angle,
                     angle_interval.end + delta_angle);
    height = delta_angle;
  }
  std::string str() {
    return (boost::format("(%1%, %2%)") % height % range.str()).str();
  }
};

std::vector<Cup> compute_cups(std::vector<Segment_2> &segments, Circle_2 c,
                              double aspect) {
  std::vector<Cup> result;
  result.reserve(segments.size());
  std::transform(segments.begin(), segments.end(), std::back_inserter(result),
                 [&](const Segment_2 &s) { return Cup(s, c, aspect); });
  return result;
}

std::vector<Cup> compute_shifted_cups(const std::vector<Cup> &cups,
                                      double amount) {
  std::vector<Cup> result;
  result.reserve(cups.size());
  std::transform(cups.begin(), cups.end(), std::back_inserter(result),
                 [&](const Cup &cup) {
                   Cup shifted_cup(cup);
                   shifted_cup.range.begin += amount;
                   shifted_cup.range.end += amount;
                   return shifted_cup;
                 });
  return result;
}

std::vector<Cup> compute_all_cups(std::vector<Segment_2> &segments, Circle_2 c,
                                  double aspect) {
  auto cups = compute_cups(segments, c, aspect);
  auto left_cups = compute_shifted_cups(cups, -2 * M_PI);
  auto right_cups = compute_shifted_cups(cups, 2 * M_PI);
  cups.insert(cups.end(), left_cups.begin(), left_cups.end());
  cups.insert(cups.end(), right_cups.begin(), right_cups.end());
  return cups;
}

std::vector<Point_2> high_points(std::vector<Cup> &cups) {
  std::vector<Point_2> high_points;
  std::vector<Interval> intervals = {{-2 * M_PI, 2 * M_PI}};
  double curr_h = 0;
  std::sort(cups.begin(), cups.end(),
            [](Cup c1, Cup c2) { return c1.height < c2.height; });
  // for (auto [height, interval] : cups) {
  for (auto cup : cups) {
    auto height = cup.height;
    auto interval = cup.range;
    double dh = height - curr_h;
    double dx = 2 * dh;
    std::vector<Interval> shrunken_intervals;
    std::for_each(intervals.begin(), intervals.end(), [&](Interval i) {
      if (i.len() < dx)
        high_points.emplace_back(i.mid(), curr_h + i.len() / 2);
      else {
        shrunken_intervals.emplace_back(i.begin + dx / 2., i.end - dx / 2.);
      }
    });
    std::vector<Interval> filtered_intervals;
    std::for_each(shrunken_intervals.begin(), shrunken_intervals.end(),
                  [&](Interval i) {
                    auto [i1, i2] = i.sub(interval);
                    if (!i1 && !i2)
                      high_points.emplace_back(i.mid(), height);
                    else {
                      if (i1)
                        filtered_intervals.push_back(*i1);
                      if (i2)
                        filtered_intervals.push_back(*i2);
                    }
                  });
    curr_h = height;
    intervals = filtered_intervals;
  }

  std::for_each(intervals.begin(), intervals.end(), [&](Interval i) {
    high_points.emplace_back(i.mid(), curr_h + i.len() / 2);
  });

  return high_points;
}

Point_2 polar_point(Circle_2 c, double angle) {
  double x = c.center().x();
  double y = c.center().y();
  double r = std::sqrt(c.squared_radius());
  return {x + r * std::cos(angle), y + r * std::sin(angle)};
}

Point_2 compute_labelling(Polygon_with_holes_2 &ph, double aspect,
                          K::Circle_2 circle) {
  std::vector<Segment_2> segments;

  auto boundary = ph.outer_boundary();
  for (auto eit = boundary.edges_begin(); eit != boundary.edges_end(); ++eit) {
    segments.push_back(*eit);
  }
  for (auto hit = ph.holes_begin(); hit != ph.holes_end(); ++hit) {
    for (auto eit = hit->edges_begin(); eit != hit->edges_end(); ++eit) {
      segments.push_back(*eit);
    }
  }
  auto cups = compute_all_cups(segments, circle, aspect);

  auto high_points_list = high_points(cups);

  std::vector<Point_2> valid_high_points;
  std::copy_if(high_points_list.begin(), high_points_list.end(),
               std::back_inserter(valid_high_points), [&](Point_2 p) {
                 return ph.outer_boundary().has_on_bounded_side(
                     polar_point(circle, p.x()));
               });

  if (valid_high_points.empty()) {
    return {0,-1};
  }

  return *std::max_element(valid_high_points.begin(), valid_high_points.end(),
                           [](Point_2 p, Point_2 q) { return p.y() < q.y(); });
}

#endif /* LABEL_FIT_HPP */
