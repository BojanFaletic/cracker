import matplotlib.pyplot as plt
import numpy as np

TIMEOUT = 0.001

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
                    d = t1-t0
                    if value == 2:
                        state = 0
                        deltas.append(d)
                    if d > TIMEOUT:
                        deltas.append(d)
                        state = 0

                t0 = t1
    except FileNotFoundError:
        print(f'File >{file}< does not exist.')
        print(f'Run fast_uart.py and save data to -> {file} <- in this dir.')
        exit(1)
    return np.array(deltas)


data = read_file()
print(f'Largest value: {np.argmax(data)}')
plt.plot(data)
plt.xlabel('Combination')
plt.ylabel('Required time [s]')
plt.savefig('read_csv.png')
plt.show()
exit(0)
