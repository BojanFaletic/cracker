import matplotlib.pyplot as plt
import numpy as np


def read_file(file='logic.csv'):
    deltas = []
    try:
        with open(file, 'r') as f:
            f.readline()
            t0 = 0
            t1 = 0
            state = 0
            while data := f.readline():
                raw_time, raw_value = data.split(',')
                t1 = float(raw_time)
                value = (~int(raw_value, 16)) & 0xF

                if state == 0:
                    if value == 1:
                        state = 1
                else:
                    if value == 2:
                        state = 0
                        deltas.append(t1-t0)

                t0 = t1
    except FileNotFoundError:
        print(f'File >{file}< does not exist.')
        print(f'Run fast_uart.py and save data to -> {file} <- in this dir.')
        exit(1)
    return np.array(deltas)


data = read_file()
print(f'Largest value: {np.argmax(data)}')
plt.plot(data)
plt.show()
exit(0)
