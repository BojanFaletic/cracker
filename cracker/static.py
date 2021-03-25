from cracker.constants import IDLE


class Static:
    def __init__(self):
        self.state = IDLE
        self.position = 0
        self.prev_time = 0
        self.deltas = {}

        for i in range(256):
            self.deltas[i] = []
