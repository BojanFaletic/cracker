from .debug_util import check_digit, brute_force
from .node import Node, depth, next_unexplored_paths, history, set_explored

# from .digit import check_digit
# from .uart_util import brute_force


def crack(nd: Node):
    key, _ = key_from_histor(nd)
    brute_force(key)


def key_from_histor(nd: Node):
    hist = history(nd)
    depth = len(hist)
    key = [0] * 7
    for i, el in enumerate(hist):
        key[i] = el
    return key, depth


def connect_node(nd: Node):
    key, depth = key_from_histor(nd)
    most_likely = check_digit(key, depth)
    for el in most_likely:
        nd_new = Node(el)
        nd.connect_next(nd_new)


def explore(nd: Node, max_depth):
    for path in next_unexplored_paths(nd):
        depth_search(path, max_depth)


def depth_search(nd: Node, max_depth=5):
    if depth(nd) == max_depth+1:
        crack(nd)
        set_explored(nd)
        return
    connect_node(nd)
    explore(nd, max_depth)


def search():
    nd = Node(-1)
    depth_search(nd)
