#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>

#include "liblabeling.h"

using std::cin;
using std::cout;
using std::endl;

liblabel::Polyline assemblePolygon(std::vector<double>& coords) {
    assert(coords.size() % 2 == 0);
    std::vector<liblabel::Point> points;
    for(auto it = coords.begin(), end = coords.end(); it != end; ++it) {
        points.push_back({*it, *++it});
    }
    return {points};
}

int main() {
    cout << "Let's compute a curved area label!" << endl;

    cout.precision(16);

    double aspect;
    cout << "Give the label aspect ratio A = H/W: ";
    cin >> aspect;
    std::cin.ignore();      // ignore the newline character after aspect input

    std::vector<double> coords;
    double tmp;
    std::string szTmp;
    cout << "Give the sequence of coordinates defining the outer boundary: " << endl;
    std::getline(cin, szTmp);
    std::istringstream szStream(szTmp);
    std::copy(std::istream_iterator<double>(szStream),
        std::istream_iterator<double>(),
        std::back_inserter(coords));
    liblabel::Polyline outer = assemblePolygon(coords);

    std::vector<liblabel::Polyline> holes;
    char input = 'i';
    while(input == 'i') {
        cout << "Enter i to insert a hole (c to continue): ";
        cin >> input;
        std::cin.ignore();      // ignore the newline character after aspect input
        if(input != 'i') continue;
        coords.clear();
        cout << "Give the sequence of coordinates defining the boundary of the hole: " << endl;
        std::getline(cin, szTmp);
        szStream = std::istringstream(szTmp);
        std::copy(std::istream_iterator<double>(szStream),
            std::istream_iterator<double>(),
            std::back_inserter(coords));
        holes.push_back(assemblePolygon(coords));
    }
    liblabel::Polygon inputPoly = {outer, holes};

    std::cout << "Starting labeling ..." << std::endl;
    auto labelOp = liblabel::computeLabel(aspect, inputPoly);

    if(labelOp.has_value()) {
        auto label = labelOp.value();
        cout << "Computed label was:\n"
            << "Center\t" << "(" << label.center.x << ", " << label.center.y << ")\n"
            << "Radii \t" << "low: " << label.rad_lower << " up: " << label.rad_upper << "\n"
            << "Angles\t" << "from: " << label.from << " to: " << label.to << endl;
        cout << "As tuple: " << endl;
        cout << "(" << label.center.x << ", " << label.center.y
             << ", " << label.rad_lower << ", " << label.rad_upper
             << ", " << label.from << ", " << label.to << ")"
             << endl;
    } else {
        cout << "Label for the given input could not be constructed!" << endl;
    }
    return 0;
}