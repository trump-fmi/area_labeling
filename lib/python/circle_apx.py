import numpy as np
from scipy import optimize
from .c_circle_apx import circle_apx, circle_apx_nl


def apx_circle(points):

    points = np.array(points)
    x = points[:, 0]
    y = points[:, 1]
    x_m = x.mean()
    y_m = y.mean()

    def calc_R(xc, yc):
        """ calculate the distance of each 2D points from the center (xc, yc) """
        return np.sqrt((x-xc)**2 + (y-yc)**2)

    def f_2(c):
        """ calculate the algebraic distance between the data points and the mean circle centered at c=(xc, yc) """
        Ri = calc_R(*c)
        return Ri - Ri.mean()

    center_estimate = x_m, y_m
    center_2, ier = optimize.leastsq(f_2, center_estimate)

    xc_2, yc_2 = center_2
    Ri_2 = calc_R(*center_2)
    R_2 = Ri_2.mean()
    return (xc_2, yc_2), R_2


apx_circle_org = apx_circle


def show_apx(points):
    from matplotlib import pyplot as plt
    from matplotlib.patches import Arc
    (x1, y1), r1 = apx_circle_org(points)
    (x2, y2), r2 = circle_apx(points)
    (x3, y3), r3 = circle_apx_nl(points)
    fig, ax = plt.subplots()
    ax.add_patch(Arc((x1, y1), 2*r1, 2*r1, color='g'))
    ax.add_patch(Arc((x2, y2), 2*r2, 2*r2, color='r'))
    ax.add_patch(Arc((x3, y3), 2*r3, 2*r3, color='b'))
    ax.scatter(*zip(*points))
    plt.axis('equal')
    plt.show()


apx_circle = circle_apx_nl
#apx_circle = show_apx
