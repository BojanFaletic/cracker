import os
import logging
from cracker import search

logging.basicConfig(filename='log/program.log', encoding='utf-8',
                    level=logging.INFO)


def create_directories() -> None:
    required_dir = ['log', 'log/temp', 'log/plots', 'log/numpy']
    for r_dir in required_dir:
        if not os.path.exists(r_dir):
            os.makedirs(r_dir)


def init_log() -> None:
    logging.info('******* START logging ******* ')


def run() -> None:
    try:
        search()
    except KeyboardInterrupt:
        print('Exiting')
    finally:
        logging.info('******* STOP logging ******* ')


def main() -> None:
    init_log()
    create_directories()
    run()


if __name__ == "__main__":
    main()
