import os
os.environ['TEST'] = '1'
from cracker.search import Graph, depth_search


class Graph_debug(Graph):
    def __init__(self, max_depth, filename):
        super(Graph_debug, self).__init__(max_depth, filename)

    def save(self):
        pass

    def load(self):
        pass


def test_node_visited():
    graph = Graph_debug(3, 'tree/tree')
    depth_search(graph)



if __name__ == "__main__":
    test_node_visited()