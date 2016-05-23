import serial

gps = serial.Serial('/dev/ttyACM0', 115200)

while True:
    print(gps.readline().replace('\r\n','').split(','))