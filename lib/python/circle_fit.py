import math
from shapely.geometry import LineString, Point
from .paths import path_candidates_single_scale, path_candidates_multi_scale
from .circle_apx import apx_circle
from .label_fit import Labeling
from .c_longest_paths import LongestPaths


def point_angle(C, p):
    return math.atan2(p[1]-C[1], p[0] - C[0])


def base_point(path, center):
    def euc_dist(p1, p2):
        x1, y1 = p1
        x2, y2 = p2
        return math.sqrt((x1-x2)**2 + (y1-y2)**2)
    dist = sum(map(lambda t: euc_dist(t[0], t[1]), zip(path, path[1:]))) / 2
    d = 0
    l = path[0]
    for c in path[1:]:
        d += euc_dist(l, c)
        if d == dist:
            return c
        elif d > dist:
            return (l[0] + c[0])/2, (l[1] + c[1])/2
        l = c


def seg_circle_dist(s, c, r):
    s = LineString(s)
    c = Point(*c)
    ds = s.distance(c)
    d0 = LineString([c, s.coords[0]]).length
    d1 = LineString([c, s.coords[1]]).length
    if all(x > r for x in [ds, d0, d1]):
        return ds - r
    if all(x < r for x in [ds, d0, d1]):
        return r - max(d0, d1)
    return 0


def placement_len(c, r, h, a, b):
    return (r-h)*(b-a)


def compute_placement(p, EDGES, ASPECT, BASELINE_HEIGHT):
    c, r = apx_circle(p)
    bp = base_point(p, c)
    base_alpha = point_angle(c, bp)

    baseline_offset = 0.5 - \
        BASELINE_HEIGHT if math.sin(base_alpha) >= 0 else BASELINE_HEIGHT-0.5

    def angle_to(p):
        a = abs(base_alpha - point_angle(c, p))
        if a > math.pi:
            return 2*math.pi - a
        return a

    def min_angle(s):
        return min(angle_to(s[0]), angle_to(s[1]))

    alpha = float('inf')
    # for s in segments:
    for s in EDGES:
        h = seg_circle_dist(s, c, r)
        s_alpha = h / (ASPECT * (r - (baseline_offset)*h))
        #s_alpha = seg_circle_dist(s, c, r)/r /ASPECT
        s_alpha = max(s_alpha, min_angle(s))
        alpha = min(alpha, s_alpha)

    h = (ASPECT * alpha * r) / (1 + ASPECT * baseline_offset * alpha)
    r_low = r - h
    r_high = r + h

    return c, r, h, base_alpha-alpha, base_alpha+alpha


def compute_placement_wedges(p, EDGES, ASPECT, BASELINE_HEIGHT):
    a, H = find_placement(EDGES, (c, r), ASPECT)
    h = H/2
    L = H/ASPECT
    da = L/(r-h)/2
    return c, r, h, a-da, a+da


def get_labelling(EDGES, GRAPH, LABEL):
    ASPECT = 1.63 / len(LABEL)
    BASELINE_HEIGHT = 0.21

    lp = LongestPaths()
    lp.build([((px, py), (qx, qy), w, c) for px, py, qx, qy, w, c in GRAPH])
    candidates = lp.paths(ASPECT, 2., 10)

    circles = [apx_circle(p) for p in candidates]
    labeller = Labeling(EDGES)
    placements = []
    for c in circles:
        a, H = labeller.find_placement(c, ASPECT)
        h = H/2
        L = H/ASPECT
        da = L/(c[1]-h)/2
        placements.append((*c, h, a-da, a+da))

    return max(placements, key=lambda p: placement_len(*p))
