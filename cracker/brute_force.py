from serial import Serial
from cracker import PORT, BAUD
from cracker.send import try_combination, is_unlocked
import logging


def brute_force(key: list) -> None:
    # search last 2 bytes with brute force
    serial = Serial(port=PORT, baudrate=BAUD, timeout=0.1)

    for byte_6 in range(256):
        for byte_7 in range(256):
            key[5] = byte_6
            key[6] = byte_7

            try_combination(serial, key)
            if is_unlocked(serial):
                print(f'UNLOCKED: with key {key}')
                logging.info(f'UNLOCKED: key: {key}')
                print('Terminating')
                serial.close()
                exit(0)
    serial.close()
