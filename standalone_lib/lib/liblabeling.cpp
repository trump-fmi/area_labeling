#include <algorithm>
#include <iostream>
#include <math.h>

#include "liblabeling.h"

#include "circle_apx.hpp"
#include "label_fit.hpp"
#include "longest_paths.hpp"
#include "segments_to_graph.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>


namespace debug {
    using namespace std;
    void printPolyline(liblabel::Polyline& pl) {
        for(auto p : pl.points) {
            cout << "(" << p.x << "," << p.y << "), ";
        }
    }
    void printPolygon(liblabel::Polygon& poly) {
        cout << "Outer boundary:" << endl;
        printPolyline(poly.outer);
        for(auto hole : poly.holes) {
            cout << "Hole: " << endl;
            printPolyline(hole);
        }
        cout << endl;
    }
}

namespace {
    // CGAL
    using K = CGAL::Exact_predicates_inexact_constructions_kernel;
    using KPoint = K::Point_2;
    using KSegment = K::Segment_2;
    using KPolygon = CGAL::Polygon_2<K>;
    using KPolyWithHoles = CGAL::Polygon_with_holes_2<K>;

    struct AugmentedSkeletonEdge {
        liblabel::Point src, trgt;
        double dist, clear;
    };

    using Path = std::vector<liblabel::Point>;
    using Skeleton = std::vector<AugmentedSkeletonEdge>;

    KPolyWithHoles constructPolygon(const liblabel::Polygon& poly);

    std::optional<std::vector<AugmentedSkeletonEdge>> constructSkeleton(const KPolyWithHoles&);

    std::vector<Path> computeLongestPaths(const std::vector<AugmentedSkeletonEdge>&, const liblabel::Aspect, const liblabel::Config&);

    std::optional<liblabel::AreaLabel> evaluatePaths(const std::vector<Path>&, const liblabel::Aspect, const KPolyWithHoles&);
}

std::optional<liblabel::AreaLabel> liblabel::computeLabel(
        liblabel::Aspect aspect,
        Polygon& poly,
        liblabel::Config configuration
    ){
    std::cout << "Starting label search with config values:\n"
        << "Step size:\t" << configuration.stepSize << "\n"
        << "Path number:\t" << configuration.numberOfPaths
        << std::endl;
    debug::printPolygon(poly);

    std::cout << "Constructing the polygon ..." << std::endl;
    KPolyWithHoles ph = constructPolygon(poly);
    std::cout << "... finished" << std::endl;

    // Construct the skeleton
    std::cout << "Construncting the skeleton ..." << std:: endl;
    auto skeletonOp = constructSkeleton(ph);
    std::cout << "... finished" << std:: endl;
    if(!skeletonOp.has_value()) {
        return {};
    }

    // Find candidate paths
    auto paths = computeLongestPaths(skeletonOp.value(), aspect, configuration);

    // Evaluate paths
    return evaluatePaths(paths, aspect, ph);
}


namespace {
    KPolyWithHoles constructPolygon(const liblabel::Polygon& poly) {
        std::vector<KPoint> tempPts;
        std::transform(poly.outer.points.begin(), poly.outer.points.end(),
            std::back_inserter(tempPts),
            [](liblabel::Point p) -> KPoint { return {p.x, p.y}; });
        KPolygon outer(tempPts.begin(), tempPts.end());
        std::vector<KPolygon> holes;
        for(liblabel::Polyline hole : poly.holes){
            tempPts.clear();
            std::transform(hole.points.begin(), hole.points.end(),
                std::back_inserter(tempPts),
                [](liblabel::Point p) -> KPoint { return {p.x, p.y}; });
            holes.emplace_back(tempPts.begin(), tempPts.end());
        }
        return KPolyWithHoles(outer, holes.begin(), holes.end());
    }

    std::optional<std::vector<AugmentedSkeletonEdge>> constructSkeleton(const KPolyWithHoles& ph) {
        if(ph.outer_boundary().size() == 0) {
            return {};
        }

        std::vector<KSegment> cgal_segs;
        std::copy(ph.outer_boundary().edges_begin(),
            ph.outer_boundary().edges_end(),
            std::back_inserter(cgal_segs));
        for(auto hit = ph.holes_begin(), end = ph.holes_end(); hit != end; ++hit) {
            std::copy(hit->edges_begin(), hit->edges_end(),
                std::back_inserter(cgal_segs));
        }

        CDT cdt(cgal_segs.begin(), cgal_segs.end());
        if(!cdt.is_valid()) {
            return {};
        }
        auto skeleton_edges = compute_skeleton_edges(cdt);

        std::cout << "Test" << std::endl;

        std::vector<AugmentedSkeletonEdge> res;
        std::transform(skeleton_edges.begin(), skeleton_edges.end(),
            std::back_inserter(res),
            [](SkeletonEdge e) -> AugmentedSkeletonEdge { return {{e.p.x(), e.p.y()}, {e.q.x(), e.q.y()}, CGAL::squared_distance(e.p, e.q), e.d};});
        return res;
    }

    std::vector<Path> computeLongestPaths(const std::vector<AugmentedSkeletonEdge>& augSkelEdges, const liblabel::Aspect aspect, const liblabel::Config& config) {
        std::vector<longest_paths::Segment> segs;
        std::transform(augSkelEdges.begin(), augSkelEdges.end(),
            std::back_inserter(segs),
            [](AugmentedSkeletonEdge e) -> longest_paths::Segment { return {{e.src.x, e.src.y}, {e.trgt.x, e.trgt.y}, e.dist, e.clear};});
        auto graph = from_edges(segs);

        auto paths = find_distinct_paths(graph, aspect, config.stepSize, config.numberOfPaths);
        
        std::vector<Path> res;
        std::transform(paths.begin(), paths.end(),
            std::back_inserter(res),
            [&graph](auto path) -> Path {
                Path res;
                std::transform(path.begin(), path.end(),
                std::back_inserter(res),
                [&graph](auto p) -> liblabel::Point {return {graph[p].x, graph[p].y};});
                return res;
            });
        return res;
    }

    std::optional<KPoint> computeAreaLabel(const circle_apx_nsp::Circle& c, const liblabel::Aspect aspect, std::vector<K::Segment_2>& segments, const KPolyWithHoles& ph) {
        K::Circle_2 circle = {{c.x, c.y}, c.r*c.r};
        auto cups = compute_all_cups(segments, circle, aspect);

        auto high_points_list = high_points(cups);

        std::vector<K::Point_2> valid_high_points;
        std::copy_if(high_points_list.begin(), high_points_list.end(),
                    std::back_inserter(valid_high_points), [&](Point_2 p) {
                        return ph.outer_boundary().has_on_bounded_side(
                            polar_point(circle, p.x()));
                    });

        if (valid_high_points.empty()) {
            return {};
        }

        return *std::max_element(valid_high_points.begin(), valid_high_points.end(),
                                [](Point_2 p, Point_2 q) { return p.y() < q.y(); });
    }

    double normalizeAngle(double angle) {
        angle = fmod(angle, 2*M_PI);
        if(angle > 2*M_PI) {
            angle -= 2*M_PI;
        }
        return angle;
    }

    liblabel::AreaLabel constructLabel(const std::pair<circle_apx_nsp::Circle, KPoint> value, const liblabel::Aspect aspect) {
        liblabel::Point center{value.first.x, value.first.y};
        double baseRadius = value.first.r;

        double baseAngle = value.second.x();
        double angleRange = value.second.y();
        double x = (aspect * angleRange);
        double h = x * center.y / (1 + x);
        // double base_angle = normalizeAngle(base_angle);
        return {
            center, baseRadius - h, baseRadius + h,
            normalizeAngle(baseAngle - angleRange),
            normalizeAngle(baseAngle + angleRange) 
        };
    }

    std::optional<liblabel::AreaLabel> evaluatePaths(const std::vector<Path>& paths, const liblabel::Aspect aspect, const KPolyWithHoles& ph) {
        std::vector<K::Segment_2> cgal_segs;
        std::copy(ph.outer_boundary().edges_begin(),
            ph.outer_boundary().edges_end(),
            std::back_inserter(cgal_segs));
        for(auto hit = ph.holes_begin(), end = ph.holes_end(); hit != end; ++hit) {
            std::copy(hit->edges_begin(), hit->edges_end(),
                std::back_inserter(cgal_segs));
        }

        std::vector<std::pair<circle_apx_nsp::Circle, KPoint>> result;
        for(auto path : paths) {
            std::vector<circle_apx_nsp::Point> points;
            std::transform(path.begin(), path.end(),
                std::back_inserter(points),
                [](liblabel::Point p) -> circle_apx_nsp::Point { return {p.x, p.y};});

            auto circle = apx_circle(points);

            auto label = computeAreaLabel(circle, aspect, cgal_segs, ph);
            if(label.has_value()) {
                result.emplace_back(circle, label.value());
            }
        }

        if(result.size() == 0) {
            return {};
        }

        auto max = std::max_element(result.begin(), result.end(),
            [](auto p, auto q) { return p.second.y() < q.second.y();} );


        return {constructLabel(*max, aspect)};
    }
}