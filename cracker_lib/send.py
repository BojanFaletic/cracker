from time import sleep
from saleae import Saleae
from serial import Serial
import logging


PORT = 'COM3'
BAUD = 115200


def write(ser, hex_number):
    tmp = bytearray()
    tmp.append(hex_number)
    ser.write(tmp)


def read(ser, length):
    return ser.read(length)


def try_combination(ser: Serial, combination: list) -> None:
    HEADER_KEY = bytearray([0xf5, 0xdf, 0xff, 0x00, 0x07])
    for byte in HEADER_KEY:
        write(ser, byte)

    for byte in combination:
        write(ser, byte)


def is_unlocked(ser: Serial) -> bool:
    write(ser, 0x70)
    read = ser.read(2)
    if len(read) == 0:
        logging.info('WARNING: UART is empty, error?')
        return False
    if read[0] == 0x80 and read[1] == 0x04:
        return False
    if read[0] == 0x80 and read[1] == 0x0c:
        logging.info("UNLOCKED!")
        return True
    else:
        logging.info(f"WARNING: Received unknown code: {read[0], read[1]}")
        return False


def check_combination(key: list):
    serial = Serial(port=PORT, baudrate=BAUD, timeout=0.1)

    # connect to logic analyzer
    logic = Saleae(quiet=True)
    logic.set_capture_seconds(170)

    logic.capture_start()
    for _ in range(1024):
        try_combination(serial, key)
        serial.flush()
        if is_unlocked(serial):
            logging.info(f'Found Key: {key}')
            break
    sleep(2)
    logic.capture_stop()
    sleep(2)
    logic.export_data2('data/logic_data.csv')
    sleep(3)
