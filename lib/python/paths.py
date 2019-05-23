import networkx as nx
import random
import numpy as np
import itertools as it
from .skeleton import filtered_graph
from shapely.geometry import LineString


def apx_norm_path(path, point_cnt=20):
    ls = LineString(path)
    steps = np.linspace(0, 1, point_cnt)
    coords = (ls.interpolate(step, normalized=True)
              for step in steps)
    return [(p.x, p.y) for p in coords]


def most_distant_node(G, sources):
    dists = nx.multi_source_dijkstra_path_length(G, sources)
    n, d = max(dists.items(), key=lambda item: item[1])
    return n if d > 0 else None


def longest_path(G, source=None):
    if source is None:
        random_node = random.choice(list(G.nodes))
        source = most_distant_node(G, [random_node])
    target = most_distant_node(G, [source])
    return tuple(nx.dijkstra_path(G, source, target)), nx.dijkstra_path_length(G, source, target)


def long_paths_comp(G, k=3):
    path, l = longest_path(G)
    yield path, l
    sources = set(path)
    for _ in range(k-1):
        nextnode = most_distant_node(G, sources)
        if not nextnode:
            break
        next_path, l = longest_path(G, nextnode)
        yield next_path, l
        sources.update(next_path)


def long_paths(G, k=3):
    paths = it.chain.from_iterable(long_paths_comp(
        Gc, k) for Gc in nx.connected_component_subgraphs(G))
    return list(it.islice(sorted(paths, key=lambda pl: pl[1], reverse=True), k))


def path_candidates_single_scale(GRAPH, num, ASPECT):
    CURRD = max(d for u, v, d in GRAPH.edges(data='clearing'))
    p, l = longest_path(filtered_graph(GRAPH, CURRD))
    while CURRD/l > ASPECT:
        CURRD = CURRD/1.4
        p, l = longest_path(filtered_graph(GRAPH, CURRD))
    path_candidates = (p for p, l in long_paths(
        filtered_graph(GRAPH, CURRD), num))
    return map(apx_norm_path, path_candidates)


def long_paths_limited(G, min_length, max_k=3):
    path, l = longest_path(G)
    if l < min_length:
        return
    yield l, path
    sources = set(path)
    for _ in range(max_k-1):
        nextnode = most_distant_node(G, sources)
        if not nextnode:
            break
        next_path, l = longest_path(G, nextnode)
        if l < min_length:
            return
        yield l, next_path
        sources.update(next_path)


def path_candidates_multi_scale(GRAPH, k, ASPECT, STEPSIZE=2):
    CURRD = max(d for u, v, d in GRAPH.edges(data='clearing'))
    cands = []
    for _ in range(10):
        min_length = CURRD / ASPECT
        Gf = filtered_graph(GRAPH, CURRD)
        paths = (it.chain.from_iterable(long_paths_limited(
            Gc, min_length, k) for Gc in nx.connected_component_subgraphs(Gf) if len(Gc.edges) > 1))
        cands.extend(paths)
        CURRD /= STEPSIZE
        if len(cands) > k:
            break
#    cands.sort(key=lambda lp: lp[0], reverse=True)
#    path_candidates = (apx_norm_path(p) for l, p in cands)
#    return it.islice(path_candidates, k)
    path_candidates = (apx_norm_path(p) for l, p in cands)
    return path_candidates
