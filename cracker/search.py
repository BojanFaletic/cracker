from cracker.digit import check_digit
from cracker.brute_force import brute_force
from graph import Graph
import logging


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

    # if no path exist start from beginning
    if depth == 0:
        # find most probable codes and add them to number
        possible_paths = check_digit(key, depth)
        gr.add_possible_paths(depth, possible_paths)

    # depth first search on unexplored paths
    for path in gr.unexplored_paths(depth):
        if path is None:
            return

        # find possible path from this position
        possible_paths = check_digit(key, depth)
        gr.add_possible_paths(depth, possible_paths)

        key[depth] = path
        print(f'Exploring path: {key}')
        logging.info(f'Exploring path: {key}')

        gr.add_explored_path(depth, path)
        depth_search(gr, depth+1)
