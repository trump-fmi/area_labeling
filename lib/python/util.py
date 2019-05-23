from shapely.geometry import Point, LineString
from math import atan2


def seg_circle_dist(segment, circle):
    segment = LineString(segment)
    p, q = map(Point, segment.coords)
    center, radius = circle
    center_point = Point(center)
    ds = segment.distance(center_point)
    dp = center_point.distance(p)
    dq = center_point.distance(q)
    if ds > radius:
        return ds - radius
    if max(dp, dq) < radius:
        return radius - max(dp, dq)
    return 0


def point_angle(p, q):
    px, py = p
    qx, qy = q
    return atan2(qy-py, qx-px)


def interval_sub(interval1, interval2):
    a1, b1 = interval1
    a2, b2 = interval2
    i1 = a1, min(b1, a2)
    i2 = max(b2, a1), b1
    result = []
    if i1[0] < i1[1]:
        result.append(i1)
    if i2[0] < i2[1]:
        result.append(i2)
    return result
