import numpy as np
from heapq import nlargest
from cracker.static import Static
import logging


def is_data_idx_valid(st: Static) -> bool:
    if st.position != 255:
        print(f'Index of csv file is invalid: {st.position}')
        return False
    return True


def is_data_length_valid(st: Static) -> bool:
    for digit_id in range(256):
        if len(st.deltas[digit_id]) < 1:
            return False
    return True


def is_data_valid(st: Static) -> bool:
    return is_data_idx_valid(st) and is_data_length_valid(st)


def print_data_summary(d: np.array) -> None:
    min_v = np.mean(d.T[0])
    max_v = np.mean(d.T[1])
    avg_v = np.mean(d.T[2])
    div_v = np.mean(d.T[3])

    logging.info(f'''\
        min: {min_v:10.3E}, max: {max_v:10.3E},
        average: {avg_v:10.3E}, deviation: {div_v:10.3E}''')


def process_deltas(st: Static) -> np.array:
    data = np.zeros((256, 4))
    for digit_id in range(256):
        min_value = np.min(st.deltas[digit_id])
        max_value = np.max(st.deltas[digit_id])
        average_value = np.mean(st.deltas[digit_id])
        divination = np.std(st.deltas[digit_id])

        data[digit_id] = [min_value, max_value, average_value, divination]

    print_data_summary(data)
    return data


def most_probable_keys(data_: np.array) -> list:
    POSSIBLE_KEYS = 3
    average = data_.T[2]
    return nlargest(POSSIBLE_KEYS, range(len(average)), average.take)
