from cracker.constants import Static
from cracker.data_util import is_data_valid, process_deltas, most_probable_keys
from cracker.csv_util import read_csv
from cracker.uart_util import run_sweep
from cracker.save_util import save_data
import logging


def key_to_label(key: list) -> str:
    key_id = ''
    for i in range(7):
        key_id += str(key[i])
        if i < 6:
            key_id += '_'
    return key_id


def check_digit(key_: list, digit_: int) -> None:
    MAX_TRES = 3

    idx = 0
    while True:
        idx += 1
        run_sweep(key_, digit_)
        static = Static()
        read_csv(static)

        if is_data_valid(static):
            break
        else:
            print('Warning CSV file not decoded correctly')
            logging.warn(f'CSV file not decoding correctly. Try num: {idx}')
            if idx > MAX_TRES:
                print('Error: Unable parse CSV file')
                raise IndexError

    data = process_deltas(static)

    label = key_to_label(key_)
    save_data(data, label)
    possible_keys = most_probable_keys(data)

    logging.info(f"Full swap done: {key_}, depth: {digit_}")
    logging.info(f'Done most likely values: {possible_keys}')
    print(f"Done cracking byte: {round(100*digit_/5)}% best case")
    return possible_keys
