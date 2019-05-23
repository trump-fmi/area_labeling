import networkx as nx
import math

from .c_segments_to_graph import to_graph


def voronoi_graph(edges_in):
    edges = to_graph(edges_in)
    return edges

    G = nx.Graph()
    G.add_edges_from(((ux, uy),
                      (vx, vy),
                      {'weight': d, 'clearing': c}) for ux, uy, vx, vy, d, c in edges)
    return G


def filtered_graph(graph, min_clearing):
    subG = nx.edge_subgraph(graph, ((u, v) for u, v, c in graph.edges(data='clearing')
                                    if c >= min_clearing))
    return subG  # max(nx.connected_component_subgraphs(subG), key=len)
