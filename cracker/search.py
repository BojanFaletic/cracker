from cracker.check_hist import check_byte
from cracker.brute_force import brute_force
import logging
import pickle


class Graph:
    def __init__(self, max_depth, filename):
        self.graph_filename = filename + '_graph.pkl'
        self.visited_filename = filename + '_visited.pkl'

        self.max_depth = max_depth
        self.graph = {}
        self.visited_graph = {}
        for i in range(max_depth):
            self.graph[i] = []
            self.visited_graph[i] = []

    def add_possible_paths(self, depth, paths):
        self.graph[depth].append(paths)

    def add_explored_path(self, depth, path):
        self.visited_graph[depth].append(path)

    def unexplored_paths(self, depth):
        for el in self.graph[depth]:
            if not (el in self.visited_graph[depth]):
                return el

    def save(self):
        with open(self.graph_filename, 'wb') as f:
            pickle.dump(self.graph, f)
        with open(self.visited_filename, 'wb') as f:
            pickle.dump(self.visited_graph, f)

    def load(self):
        try:
            with open(self.graph_filename, 'rb') as f:
                self.graph = pickle.load(f)
        except FileNotFoundError:
            print('Graph not found, starting fresh')

        try:
            with open(self.visited_filename, 'rb') as f:
                self.visited_graph = pickle.load(f)
        except FileNotFoundError:
            print('Visited graph not found, starting fresh')


def depth_search(gr: Graph, depth=0, key=[0]*7):
    if depth == gr.max_depth:
        # save graph
        gr.save()

        # brute force last 2 bytes
        print('Brute forcing last 2 bytes')
        logging.info('Brute forcing last 2 bytes')
        brute_force(key)
        logging.info(f'Brute forcing unsuccessfull: {key}')
        return

    # find most probable codes and add them to number
    possible_paths = check_byte(key, depth)
    gr.add_possible_paths(depth, possible_paths)

    # depth first search on unexplored paths
    for path in gr.unexplored_paths(depth):
        key[depth] = path
        print(f'Exploring path: {key}')
        logging.info(f'Exploring path: {key}')

        gr.add_explored_path(depth, path)
        depth_search(gr, depth+1)
