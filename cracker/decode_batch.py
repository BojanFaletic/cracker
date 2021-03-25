#from cracker.decode_data import to_signal
#import logging

TX = 1
RX = 2
IDLE = 0

def to_signal(value_: int) -> int:
    return (~value_) & 0xF

def decode_batch() -> list:
    file_name = 'experiments/sequence_batch/test.csv'
    deltas = []
    data_dict = {}
    state = 0
    time_prev = 0
    digit = 0
    with open(file_name, 'r') as f:
        header = f.readline()
        if header != 'Time[s], Data[Hex]\n':
            logging.info(f'Warning heder not match: {header}')
        while data := f.readline():
            raw_time, raw_value = data.split(',')

            # decode current time, value from file
            time = float(raw_time)
            value = to_signal(int(raw_value, 16))

            # if long pause, move to next digit
            delta = time - time_prev
            if delta >= 0.2:
                data_dict[digit] = deltas
                deltas = []
                print('Here!')
                digit += 1

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
            else:
                print('error undefined state')

            time_prev = time
    return data_dict



if __name__ == "__main__":

    print(decode_batch())