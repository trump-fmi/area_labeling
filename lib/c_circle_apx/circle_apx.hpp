#ifndef CIRCLE_APX_HPP
#define CIRCLE_APX_HPP

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include <nlopt.hpp>

namespace circle_apx_nsp {
  struct Point {
    double x, y;
  };

  struct Circle {
    double x, y, r;
  };
}

circle_apx_nsp::Circle apx_circle(const std::vector<circle_apx_nsp::Point> &points) {
  size_t N = points.size();

  double sx =
      std::accumulate(points.begin(), points.end(), 0.,
                      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.x; });
  double sy =
      std::accumulate(points.begin(), points.end(), 0.,
                      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.y; });

  double sx2 = std::accumulate(
      points.begin(), points.end(), 0.,
      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.x * p.x; });
  double sxy = std::accumulate(
      points.begin(), points.end(), 0.,
      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.x * p.y; });
  double sy2 = std::accumulate(
      points.begin(), points.end(), 0.,
      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.y * p.y; });

  double sx3 = std::accumulate(
      points.begin(), points.end(), 0.,
      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.x * p.x * p.x; });
  double sx2y = std::accumulate(
      points.begin(), points.end(), 0.,
      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.x * p.x * p.y; });
  double sxy2 = std::accumulate(
      points.begin(), points.end(), 0.,
      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.x * p.y * p.y; });
  double sy3 = std::accumulate(
      points.begin(), points.end(), 0.,
      [](double acc, const circle_apx_nsp::Point &p) { return acc + p.y * p.y * p.y; });

  double a1 = 2 * (sx * sx - N * sx2);
  double b1 = 2 * (sx * sy - N * sxy);
  double a2 = 2 * (sx * sy - N * sxy);
  double b2 = 2 * (sy * sy - N * sy2);
  double c1 = (sx2 * sx - N * sx3 + sx * sy2 - N * sxy2);
  double c2 = (sx2 * sy - N * sy3 + sy * sy2 - N * sx2y);

  double denom = (a1 * b2 - a2 * b1);
  denom = std::max(denom, .00000000001);

  double x_bar = (c1 * b2 - c2 * b1) / denom;
  double y_bar = (a1 * c2 - a2 * c1) / denom;

  double R_squared = (sx2 - 2 * sx * x_bar + N * x_bar * x_bar + sy2 -
                      2 * sy * y_bar + N * y_bar * y_bar) /
                     N;

  if (!std::isfinite(x_bar) || !std::isfinite(y_bar)) {
    std::cout.precision(20);
    std::cout << "not finite from:" << std::endl;
    for (auto p : points) {
      std::cout << p.x << " " << p.y << std::endl;
    }
    std::cout << a1 * b2 - a2 * b1 << std::endl;
    std::cout << "not finite up" << std::endl;
  }

  return {x_bar, y_bar, std::sqrt(R_squared)};
}

std::vector<double> compute_radii(const std::vector<circle_apx_nsp::Point> &points,
                                  circle_apx_nsp::Point center) {
  std::vector<double> radii;
  std::transform(points.begin(), points.end(), std::back_inserter(radii),
                 [center](const circle_apx_nsp::Point &p) {
                   double dx = p.x - center.x;
                   double dy = p.y - center.y;
                   return std::sqrt(dx * dx + dy * dy);
                 });
  return radii;
}

std::vector<double> compute_derivative(const std::vector<circle_apx_nsp::Point> &points,
                                       circle_apx_nsp::Point center) {
  // https://core.ac.uk/download/pdf/35472611.pdf
  auto radii = compute_radii(points, center);
  auto R = std::accumulate(radii.begin(), radii.end(), 0.) / radii.size();

  double x_grad = 0.;
  double y_grad = 0.;
  for (auto p : points) {
    auto dx = center.x - p.x;
    auto dy = center.y - p.y;
    if (dx == 0 and dy == 0)
      continue;
    x_grad += 2 * (dx - R * (dx / std::sqrt(dx * dx + dy * dy)));
    y_grad += 2 * (dy - R * (dy / std::sqrt(dx * dx + dy * dy)));
  }

  return {x_grad, y_grad};
}

double err(const std::vector<double> &center, std::vector<double> &grad,
           void *points_ref) {
  const std::vector<circle_apx_nsp::Point> &points = *(const std::vector<circle_apx_nsp::Point> *)points_ref;
  double x = center[0];
  double y = center[1];

  if (!grad.empty()) {
    auto grads = compute_derivative(points, {x, y});
    grad[0] = grads[0];
    grad[1] = grads[1];
  }

  auto radii = compute_radii(points, {x, y});
  auto R = std::accumulate(radii.begin(), radii.end(), 0.) / radii.size();

  return std::accumulate(
      radii.begin(), radii.end(), 0.,
      [R](double acc, double r) { return acc + (R - r) * (R - r); });
}

circle_apx_nsp::Point apx_nl(const std::vector<circle_apx_nsp::Point> &points) {

  nlopt::opt opt(nlopt::LD_MMA, 2);
  opt.set_ftol_rel(.0001);
  opt.set_xtol_rel(.001);
  opt.set_min_objective(err, (void *)&points);

  circle_apx_nsp::Circle c = apx_circle(points);
  std::vector<double> x = {c.x, c.y}; /* `*`some` `initial` `guess`*` */
  double minf; /* `*`the` `minimum` `objective` `value,` `upon` `return`*` */
  if (points.size() > 3)
    opt.optimize(x, minf);

  return {x[0], x[1]};
}

circle_apx_nsp::Circle apx_circle_nl(const std::vector<circle_apx_nsp::Point> &points) {
  circle_apx_nsp::Point center = apx_nl(points);
  auto radii = compute_radii(points, center);
  auto R = std::accumulate(radii.begin(), radii.end(), 0.) / radii.size();
  return {center.x, center.y, R};
}

#endif /* CIRCLE_APX_HPP */
