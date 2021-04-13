from serial import Serial
from time import sleep

''' 1) Try crack single byte. Run logic for ~300 sec '''
''' 2) Save logic to CSV file (this directory) '''
''' 3) Run read_csv.py to display time '''

ser = Serial('COM3', 9600)
ser.close()

for key in range(255, 0, -1):
    ser.open()
    sleep(0.02)
     # send 16 zeros
    for _ in range(16):
        ser.write(b'\x00')
        sleep(0.03)
    # header
    ser.write(b'\xf5')
    ser.write(b'\xdf')
    ser.write(b'\xff')
    ser.write(b'\x00')
    ser.write(b'\x07')
    # key
    
    ser.write(bytes([key]))
    ser.write(b'\xff')
    ser.write(b'\xff')
    ser.write(b'\xff')
    ser.write(b'\xff')
    ser.write(b'\xff')
    ser.write(b'\xff')
    # read
    ser.write(b'\x70')
    ser.flush()
    sleep(0.02)
    ser.close()
    
