import os
import logging
from cracker_lib.search import depth_search, Graph


def create_directories():
    required_dir = ['data', 'tree', 'checkpoint', 'plots']
    for r_dir in required_dir:
        if not os.path.exists(r_dir):
            os.makedirs(r_dir)


def init_log():
    logging.basicConfig(filename='program.log', encoding='utf-8',
                        level=logging.INFO)


def run():
    graph = Graph(max_depth=5, filename='tree/tree')
    graph.load()
    depth_search(graph)


def main():
    init_log()
    create_directories()
    run()


if __name__ == "__main__":
    main()
