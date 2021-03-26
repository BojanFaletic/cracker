from cracker.static import Static
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


def is_digit_correct(fist_: list, second_: list) -> bool:
    for el in fist_:
        if el in second_:
            return True
    return False


def correct_digit(first_: list, second_: list) -> list:
    new_key = []
    for el in first_:
        if el in second_:
            new_key.append(el)

    if not first_[0] in new_key:
        new_key.append(first_[0])
    if not second_[0] in new_key:
        new_key.append(second_[0])
    return new_key


def check_correct_digit(key_: list, digit_: int) -> list:
    first_pass = check_digit(key_, digit_)

    while True:
        second_pass = first_pass
        first_pass = check_digit(key_, digit_)
        if is_digit_correct(first_pass, second_pass):
            break

    key = correct_digit(first_pass, second_pass)
    logging.info(f'Found digit: {key[0]}, depth: {digit_}')
    return key


def check_digit(key_: list, digit_: int) -> list:
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
