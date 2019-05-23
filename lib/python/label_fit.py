from .util import seg_circle_dist, point_angle, interval_sub
from shapely.geometry import Polygon, Point
import math
from .c_label_fit import LabelFitter


def seg_circle_wedge(segment, circle, ASPECT):
    ds, (a, b) = seg_circle_box(segment, circle)
    xy, r = circle
    x = ASPECT * math.pi
    h_max = x*r/(1+x)
    h = min(ds, h_max)
    rb = r - h
    H = 2*h
    L = H/ASPECT
    delta_angle = L / rb / 2

    if b-a > math.pi:
        return [(delta_angle, (b-delta_angle, a+2*math.pi+delta_angle), 1),
                (delta_angle, (b-2*math.pi-delta_angle, a+delta_angle), 1)]
    return [(delta_angle, (a-delta_angle, b+delta_angle), 1)]


def seg_circle_box(segment, circle):
    return seg_circle_dist(segment, circle), seg_circle_angles(segment, circle)


def seg_circle_angles(segment, circle):
    circle_center, _ = circle

    def center_angle(p):
        return point_angle(circle_center, p)
    p, q = segment
    return list(sorted([center_angle(p), center_angle(q)]))


def compute_wedges(circle, edges, aspect):
    wedges = []
    for s in edges:
        w = seg_circle_wedge(s, circle, aspect)
        wedges.extend(w)
    return wedges


def highest_point_under_wedges(wedges):
    ascend = wedges[0][2]
    left = -2*math.pi  # min(w[1][0] for w in zero_wedges)
    right = 2*math.pi  # max(w[1][1] for w in zero_wedges)
    assert left < right
    intervals = [(left, right)]
    curr_h = 0
    for h, (a, b), _ in sorted(wedges):
        assert a < b
        assert all(ia < ib for ia, ib in intervals)
        dh = h - curr_h
        dx = 2*dh/ascend
        shrunken_intervals = []
        for ia, ib in intervals:
            if ib - ia < dx:
                yield (ia+ib)/2, curr_h + (ib-ia)*ascend/2
            else:
                shrunken_intervals.append((ia+dx/2, ib-dx/2))
        filtered_intervals = []
        for i in shrunken_intervals:
            new_i = interval_sub(i, (a, b))
            if new_i == []:
                yield (i[0]+i[1])/2, h
            else:
                filtered_intervals.extend(new_i)
        curr_h = h
        intervals = filtered_intervals
    for ia, ib in intervals:
        yield (ia+ib)/2, curr_h + (ib-ia)*ascend/2


def shift_wedge(w, amount):
    h, (a, b), ascend = w
    return h, (a+amount, b+amount), ascend


def shift_wedges(wedges, amount):
    return [shift_wedge(w, amount) for w in wedges]


def normalize_angle(a):
    a = a % (2*math.pi)
    if a > math.pi:
        a -= 2*math.pi
    return a


class Labeling:
    def __init__(self, poly_with_holes):
        self.fitter = LabelFitter()
        poly, holes = poly_with_holes
        points = list(zip(poly[::2], poly[1::2]))
        self.fitter.build(points[1:])

    def find_placement(self, circle, aspect):
        base_angle, angle_range = self.fitter.label(aspect, circle[1], circle[0])

        angle_range *= 2
        x = aspect * angle_range / 2
        h = x*circle[1]/(1+x)
        base_angle = normalize_angle(base_angle)

        return base_angle, 2*h
