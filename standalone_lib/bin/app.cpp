#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <iterator>
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

Input streamInput() {
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

Input interactiveInput() {
    liblabel::Aspect aspect;

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
    cout << "Outer Polygon contained " << coords.size() << " many elements." << endl;
    liblabel::Polyline outer = assemblePolyline(coords);

    std::vector<liblabel::Polyline> holes;
    char input = 'i';
    while(input == 'i') {
        cout << "Enter i to insert a hole (c to cancel): ";
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
        holes.push_back(assemblePolyline(coords));
    }

    return {aspect, {outer, holes}};
}

int main(int argc, char** argv) {
    if(argc < 2) {
        cout << "Please use -i for interactive or -s for streamed input."
             << endl;
    } else if ("-i" == std::string(argv[1])) {
        cout << "Get labeling parameters interactively!" << endl;
        Input input = interactiveInput();

        cout << "Let's compute a curved area label!" << endl;
        cout << "Aspect was " << input.aspect << endl;
        cout << "Input poly outer poly has " << input.poly.outer.points.size() << " points"
            << " and " << input.poly.holes.size() << " holes." << endl;

        std::cout << "Starting labeling ..." << std::endl;
        auto labelOp = liblabel::computeLabel(input.aspect, input.poly, true);

        cout << std::setprecision(std::numeric_limits<double>::digits10 + 1);
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
            cerr << "Label for the given input could not be constructed!" << endl;
        }
    } else if ("-s" == std::string(argv[1])) {
        Input input = streamInput();

        auto labelOp = liblabel::computeLabel(input.aspect, input.poly);

        cout << std::setprecision(std::numeric_limits<double>::digits10 + 1);
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
    } else {
        cout << "Please use -i for interactive or -s for streamed input."
             << endl;
    }
    return 0;
}