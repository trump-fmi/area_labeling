#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>

#include "liblabeling.h"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

liblabel::Polyline assemblePolyline(std::vector<double>& coords) {
    assert(coords.size() % 2 == 0);
    std::vector<liblabel::Point> points;
    for(auto it = coords.begin(), end = coords.end(); it != end; ++it) {
        points.push_back({*it, *++it});
    }
    return {points};
}

struct Input {
    liblabel::Aspect aspect;
    liblabel::Polygon poly;
};

Input interactiveInput() {
    liblabel::Aspect aspect;

    cin >> aspect;
    std::cin.ignore();      // ignore the newline character after aspect input

    std::vector<double> coords;
    double tmp;
    std::string szTmp;
    std::getline(cin, szTmp);
    std::istringstream szStream(szTmp);
    std::copy(std::istream_iterator<double>(szStream),
        std::istream_iterator<double>(),
        std::back_inserter(coords));
    liblabel::Polyline outer = assemblePolyline(coords);

    std::vector<liblabel::Polyline> holes;
    for(std::string tmp; std::getline(cin, tmp);) {
        coords.clear();
        szStream = std::istringstream(tmp);
        std::copy(std::istream_iterator<double>(szStream),
            std::istream_iterator<double>(),
            std::back_inserter(coords));
        if(holes.size() == 0) {
            break;
        }
        holes.push_back(assemblePolyline(coords));
    }

    return {aspect, {outer, holes}};
}

int main(int argc, char** argv) {
    cout << std::setprecision(std::numeric_limits<double>::digits10 + 1);
    Input input = interactiveInput();

    auto labelOp = liblabel::computeLabel(input.aspect, input.poly);

    if(labelOp.has_value()) {
        auto label = labelOp.value();
        cout << "AreaLabel: "
             << "(" << label.center.x << ", " << label.center.y
             << ", " << label.rad_lower << ", " << label.rad_upper
             << ", " << label.from << ", " << label.to << ")"
             << endl;
    } else {
        cerr << "Label for the given input could not be constructed!" << endl;
    }
    return 0;
}