import numpy as np
from heapq import nlargest
from cracker.static import Static


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


def process_deltas(st: Static) -> np.array:
    data = np.zeros((256, 4))
    for digit_id in range(256):
        min_value = np.min(st.deltas[digit_id])
        max_value = np.max(st.deltas[digit_id])
        average_value = np.mean(st.deltas[digit_id])
        divination = np.std(st.deltas[digit_id])

        data[digit_id] = [min_value, max_value, average_value, divination]
    return data


def most_probable_keys(data_: np.array) -> list:
    POSSIBLE_KEYS = 3
    avg = data_.T[2]
    return nlargest(POSSIBLE_KEYS, range(len(avg)), avg.take)
