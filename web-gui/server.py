from lib.skeleton import voronoi_graph
from lib.circle_fit import get_labelling
import itertools as it
import math
import numpy as np
import cProfile, pstats
from contextlib import contextmanager

from flask import Flask, render_template, request
import json


app = Flask(__name__)


def subsample_segment(seg, k):
    (px, py), (qx, qy) = seg
    xs = np.linspace(px, qx, k)
    ys = np.linspace(py, qy, k)
    coords = list(zip(xs, ys))
    yield from zip(coords, coords[1:])


def subsample_segments(segs, N=100):
    n = len(segs)
    k = int(math.ceil(N/n))
    return it.chain.from_iterable(subsample_segment(s, k) for s in segs)


def polylines_to_segments(polylines):
    return it.chain.from_iterable(map(polyline_to_segments, polylines))


def polyline_to_segments(polyline):
    for p, q in zip(polyline, polyline[1:]):
        yield p, q


@contextmanager
def print_profile():
    pr = cProfile.Profile()
    pr.enable()
    yield
    pr.disable()
    sortby = pstats.SortKey.TIME
    ps = pstats.Stats(pr).sort_stats(sortby)
    ps.print_stats(20)


@app.route("/label", methods=['POST'])
def label():
#    try:
        data = json.loads(request.data.decode())
        segments = list(polylines_to_segments(data['poly']))
#        segments = list(subsample_segments(segments))
        with print_profile():
            graph = voronoi_graph(segments)
            poly = [x for p, q in segments for x in p]
            c, r, h, a, b = get_labelling([poly, []], graph, data['text'])
        return json.dumps({'c': c, 'r': r, 'h': h, 'a': a, 'b': b})
#    except Exception as err:
#        return f'an error occured: {err}', 500


@app.route("/skeleton", methods=['POST'])
def skeleton():
    try:
        data = json.loads(request.data.decode())
        segments = list(polylines_to_segments(data))
#        segments = list(subsample_segments(segments))
        graph = voronoi_graph(segments)
        graph = [((a, b), (c, d)) for a, b, c, d, e, f in graph]
        return json.dumps(graph)
    except Exception as err:
        return f'an error occured: {err}', 500


@app.route("/")
def hello():
    return render_template('index.html')


if __name__ == "__main__":
    app.run(host='0.0.0.0')
