import serial

ser = serial.Serial('/dev/ttyUSB0', 115200)

# header
ser.write(b'\xf5')
ser.write(b'\xdf')
ser.write(b'\xff')
ser.write(b'\x00')
ser.write(b'\x07')
# key
ser.write(b'\x00')
ser.write(b'\x00')
ser.write(b'\x00')
ser.write(b'\x00')
ser.write(b'\x00')
ser.write(b'\x00')
ser.write(b'\x00')
# read
ser.write(b'\x70')
