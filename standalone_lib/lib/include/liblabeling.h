#ifndef LIBLABELING_H
#define LIBLABELING_H

#include <optional>
#include <vector>

namespace liblabel {
    /**
     * The aspect is describing the ratio of the height to the width of a box
     * i.e. A = H / L where H is the height and L the width of the box.
     */ 
    using Aspect = double;

    /**
     * A 2-dimensional point
     */
    struct Point {
        double x, y;
    };

    /**
     * A polyline consisting of a sequence of 2d points describing their shape.
     */
    struct Polyline {
        std::vector<Point> points;
    };

    /**
     * A polygon consisting of an outer boundary.
     * Optionally containing holes.
     */
    struct Polygon {
        // The outer boundary.
        // The last point is connected to the first one to close the ring.
        Polyline outer;

        // The holes each are described by a Polyline where the last node is
        // connected to the first one to close the ring.
        std::vector<Polyline> holes;
    };

    struct Config {
        // Step size during the longest path search
        double stepSize = 2.;

        // Number of alternative longest paths to consider
        size_t numberOfPaths = 20;
    };

    struct AreaLabel {
        // center point of the label arc
        Point center;
        // upper and lower radius of the label position
        double rad_lower, rad_upper;
        // label starts at angle <from> and goes to <to>
        // both angles are in degrees where 0 is the direction of the x axis
        double from, to;
    };

    std::optional<liblabel::AreaLabel> computeLabel( liblabel::Aspect,
                                                     liblabel::Polygon&,
                                                     bool progress = false,
                                                     liblabel::Config = liblabel::Config() );
}

#endif /* LIBLABELING_H */