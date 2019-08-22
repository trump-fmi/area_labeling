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
    cout << "Hello from main!" << endl;

    double aspect;
    cout << "Give the aspect ratio: ";
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
    cout << "Got coordinate sequence: " << endl;
    for(auto c : coords) {
        cout << c << " ";
    }
    cout << endl;
    liblabel::Polyline outer = assemblePolygon(coords);

    std::vector<liblabel::Polyline> holes;
    char input = 'i';
    cout << "Press i to insert a hole: ";
    cin >> input;
    while(input == 'i') {
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
            << "Angles\t" << "from: " << label.from << " to: " << label.to
            << endl;
    } else {
        cout << "Label for the given input could not be constructed!" << endl;
    }
    return 0;
}