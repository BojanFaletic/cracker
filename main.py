import os
import logging
from cracker.graph import depth_search, Graph
logging.basicConfig(filename='program.log', encoding='utf-8',
                    level=logging.INFO)


def create_directories():
    required_dir = ['data', 'tree', 'checkpoint', 'plots']
    for r_dir in required_dir:
        if not os.path.exists(r_dir):
            os.makedirs(r_dir)


def init_log():
    logging.info('******* START logging ******* ')


def run():
    graph = Graph(max_depth=5, filename='tree/tree')
    graph.load()
    try:
        depth_search(graph)
    except KeyboardInterrupt:
        print('Exiting')
    finally:
        graph.save()
        logging.info('******* STOP logging ******* ')


def main():
    init_log()
    create_directories()
    run()


if __name__ == "__main__":
    main()
