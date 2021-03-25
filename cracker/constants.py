
# Serial configuration
PORT = 'COM3'
BAUD = 115200

# packet constants
IDLE = 0
TX = 1
RX = 2

# sending
PACKET_SIZE = 64
DIGIT_IDLE_TIME = 0.2

# Status bits
WAVEFORM_FILE = 'data/logic_data.csv'
PLOT_DIR = 'plots/'
RAW_DIR = 'checkpoints/'


class Static:
    def __init__(self):
        self.state = IDLE
        self.position = 0
        self.prev_time = 0
        self.deltas = {}

        for i in range(256):
            self.deltas[i] = []
