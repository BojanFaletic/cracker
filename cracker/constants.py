
# Serial configuration
PORT = 'COM3'
BAUD = 115200

# packet constants
IDLE = 0
TX = 1
RX = 2

# sending
PACKET_SIZE = 16
DIGIT_IDLE_TIME = 0.05

# Filename
WAVEFORM = 'logic_data.csv'
LOG = 'program_log.txt'

# Folders
TOP_FOLDER = 'log'
WAVEFORM_FOLDER = 'temp'
PLOT_FOLDER = 'plots'
NUMPY_SUMMARY = 'numpy'

# Files
WAVEFORM_FILE = TOP_FOLDER + '/' + WAVEFORM_FOLDER + '/' + WAVEFORM
PLOT_FILE = TOP_FOLDER + '/' + PLOT_FOLDER + '/'
NUMPY_FILE = TOP_FOLDER + '/' + NUMPY_SUMMARY + '/'
LOG_FILE = TOP_FOLDER + '/' + LOG
