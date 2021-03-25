from serial import Serial, SerialException
from saleae import Saleae
import logging
from copy import deepcopy
from time import sleep
from cracker.constants import PORT, BAUD, PACKET_SIZE,\
     DIGIT_IDLE_TIME, WAVEFORM_FILE


def write(ser: Serial, hex_number: int) -> None:
    tmp = bytearray()
    tmp.append(hex_number)
    ser.write(tmp)
    ser.flush()


def read(ser: Serial, length: int) -> list:
    return ser.read(length)


def try_combination(ser: Serial, combination: list) -> None:
    HEADER_KEY = bytearray([0xf5, 0xdf, 0xff, 0x00, 0x07])
    for byte in HEADER_KEY:
        write(ser, byte)

    for byte in combination:
        write(ser, byte)


def read_request(ser: Serial):
    write(ser, 0x70)
    return ser.read(2)


def is_unlocked(ser: Serial) -> bool:
    INVALID_ID = b'\x80\x04'
    VALID_ID = b'\x80\x0c'

    status = read_request(ser)
    if len(status) == 0:
        logging.warn('UART is empty, error?')
        return False
    if status == INVALID_ID:
        return False
    if status == VALID_ID:
        logging.info("UNLOCKED!")
        return True
    else:
        logging.warn(f"Received unknown code: {status}")
        return False


def connect_logic() -> Saleae:
    logic_ = Saleae(quiet=True)
    return logic_


def connect_serial() -> Serial:
    s_ = None
    try:
        s_ = Serial(port=PORT, baudrate=BAUD, timeout=0.1)
    except SerialException:
        print('Serial device not found on PORT:{PORT}')
        raise SerialException
    return s_


def check_combination(ser: Serial, key: list) -> bool:
    try_combination(ser, key)
    if is_unlocked(ser):
        print(f'Found key:{key}')
        logging.info(f'Found key:{key}')
        return True
    return False


def disconnect_serial(s: Serial) -> None:
    s.close()
    del s


def disconnect_logic(s: Saleae) -> None:
    del s


def configure_logic(log: Saleae) -> None:
    log.set_capture_seconds(1024)
    log.capture_start()
    sleep(1)


def save_data(log: Saleae) -> None:
    log.export_data2(WAVEFORM_FILE)
    sleep(3)


def check_key(ser: Serial, try_key: list) -> None:
    for idx in range(PACKET_SIZE):
        check_combination(ser, try_key)
    sleep(DIGIT_IDLE_TIME+0.1)


def brute_force(key: list) -> None:
    # search last 2 bytes with brute force
    serial = connect_serial()

    try_key = deepcopy(key)
    for byte_6 in range(256):
        for byte_7 in range(256):
            try_key[5] = byte_6
            try_key[6] = byte_7

            try_combination(serial, try_key)
            if is_unlocked(serial):
                print(f'UNLOCKED: with key {try_key}')
                logging.info(f'UNLOCKED: key: {try_key}')
                print('Terminating')
                disconnect_serial()
                exit(0)
    disconnect_serial()
    print(f'Brute force failed: {key}')


def run_sweep(key: list, digit: int) -> None:
    analyzer = connect_logic()
    ttyUSB = connect_serial()

    try_key = deepcopy(key)
    configure_logic(analyzer)

    for try_id in range(256):
        try_key[digit] = try_id
        check_key(ttyUSB, try_key)

    sleep(DIGIT_IDLE_TIME)
    analyzer.capture_stop()
    sleep(1)

    save_data(analyzer)

    disconnect_serial(ttyUSB)
    disconnect_logic(analyzer)
