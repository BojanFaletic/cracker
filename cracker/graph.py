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
        return None

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
