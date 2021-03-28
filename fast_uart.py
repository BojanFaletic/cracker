from serial import Serial
from time import sleep

''' Try crack single byte. Run logic for ~300 sec '''

ser = Serial('COM3', 9600)
ser.close()

for key in range(256):
    ser.open()
    sleep(0.2)
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
    ser.write(b'\x00')
    ser.write(b'\x00')
    ser.write(b'\x00')
    ser.write(b'\x00')
    ser.write(b'\x00')
    ser.write(b'\x00')
    # read
    ser.write(b'\x70')
    ser.flush()
    sleep(0.2)
    ser.close()
    sleep(0.2)
