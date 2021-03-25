from cracker.constants import IDLE, TX, RX, WAVEFORM_FILE, DIGIT_IDLE_TIME
from cracker.static import Static


def to_signal(value_: int) -> int:
    return (~value_) & 0xF


def wait_until_transmit(st: Static, value_: int) -> None:
    if st.state == IDLE and value_ == TX:
        st.state = TX


def save_delta(st: Static, time_: float) -> None:
    if st.position >= 0:
        delta = time_ - st.prev_time
        st.deltas[st.position].append(delta)


def wait_until_receive(st: Static, value_: int, time_: float) -> None:
    if st.state == TX and value_ == RX:
        save_delta(st, time_)
        st.state = IDLE


def select_digit(st: Static, time_: float) -> None:
    time_thr = DIGIT_IDLE_TIME * (1-0.05)
    delta = time_ - st.prev_time
    if delta >= time_thr:
        st.position += 1
    if st.position > 255:
        print('Byte position is larger than 255')
        raise IndexError


def decode_UART(st: Static, value_: int, time_: float):
    wait_until_transmit(st, value_)
    wait_until_receive(st, value_, time_)
    select_digit(st, time_)
    st.prev_time = time_


def check_csv_header(header: str) -> None:
    HEADER = 'Time[s], Data[Hex]\n'
    if header != HEADER:
        print('Wrong csv file')
        raise FileNotFoundError


def decode_line(line: str) -> (int, float):
    raw_time, raw_value = line.split(',')
    time_ = float(raw_time)
    value_ = to_signal(int(raw_value, 16))
    return value_, time_


def read_csv(st: Static, f_name=WAVEFORM_FILE) -> None:
    with open(f_name, 'r') as f:
        check_csv_header(f.readline())
        while data := f.readline():
            value, time = decode_line(data)
            decode_UART(st, value, time)
