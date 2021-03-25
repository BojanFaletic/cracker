from cracker.decode_data import decode_uart
from cracker.send import check_combination
import logging
import numpy as np
import matplotlib.pyplot as plt
import heapq


def check_byte(key: list, digit: int) -> list:
    full_combinations = np.zeros((256, 4))

    # key to string
    key_id = ''
    for i in range(7):
        key_id += str(key[i])
        if i < 6:
            key_id += '_'

    # perform full swap over range
    for combination in range(256):
        key[digit] = combination
        check_combination(key)
        deltas = decode_uart()

        np_deltas = np.array(deltas)
        min_value = np.min(np_deltas)
        max_value = np.max(np_deltas)
        average_value = np.mean(np_deltas)
        std_div = np.std(np_deltas)

        data_summary = [min_value, max_value, average_value, std_div]

        full_combinations[combination] = data_summary

    logging.info(f"Full swap done: {key}, depth: {digit}")

    # saves
    np.save(f'checkpoint/{key_id}.npy', full_combinations)

    # plots
    t = np.arange(256)
    plt.plot(t, full_combinations.T[0], t, full_combinations.T[1],
             t, full_combinations.T[2])
    plt.savefig(f'plots/{key_id}_min_max_avg.png')

    plt.plot(t, full_combinations.T[2], t, full_combinations.T[3])
    plt.savefig(f'plots/{key_id}_avg_std.png')

    # select most probable combinations
    longest_digits = full_combinations.T[1]

    # find longest combinations
    most_probable_keys = heapq.nlargest(3, range(len(longest_digits)),
                                        longest_digits.take)

    logging.info(f'Done most likely values: {most_probable_keys}')
    print(f'Done most likely values: {most_probable_keys}')
    print(f"Done cracking byte: {round(100*digit/5)}% best case")

    return most_probable_keys
