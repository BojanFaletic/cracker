import logging
from cracker import TX, RX, IDLE


def to_signal(value_: int) -> int:
    return (~value_) & 0xF


def decode_uart() -> list:
    deltas = []
    state = 0
    time_prev = 0
    with open('data/logic_data.csv', 'r') as f:
        header = f.readline()
        if header != 'Time[s], Data[Hex]\n':
            logging.info(f'Warning heder not match: {header}')
        while data := f.readline():
            raw_time, raw_value = data.split(',')

            # decode current time, value from file
            time = float(raw_time)
            value = to_signal(int(raw_value, 16))

            # calculate time from RX to TX
            if state == 0:
                if value == TX:
                    state = 1
            elif state == 1:
                if value == RX:
                    state = 2
                    delta = time - time_prev
                    deltas.append(delta)
            elif state == 2:
                if value == IDLE:
                    state = 0
            time_prev = time
    return deltas
