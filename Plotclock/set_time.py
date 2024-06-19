#/usr/bin/python

import serial
import datetime
import time
import sys

code = 'latin-1'

serial_device = '/dev/ttyUSB0'
if len(sys.argv) == 2:
    serial_device = sys.argv[1]
else:
    print (f"Usage: {sys.argv[0]} serial_device")

print (f'Using serial device "{serial_device}"')

ser = serial.Serial(serial_device, baudrate=115200, timeout=1, dsrdtr=True)
assert(ser.is_open)

print ("connected. Waiting for greeting text...")
remaining_tries = 10

line = ser.readline()
while line or remaining_tries != 0:
    if line:
        decoded_line = line.decode(code).strip()
        print (f'Host <-- Device: {decoded_line}')
        if "Date like " in decoded_line:
            print ("Got expected greeting text!")
            break
    else:
        print (f"Waiting ({remaining_tries})...")
        remaining_tries -= 1
    line = ser.readline()

if remaining_tries == 0:
    print ("Timeout. Sending anyway!")

now = datetime.datetime.now(datetime.UTC)

day = now.strftime("%b %d %Y")
wct = now.strftime("%H:%M:%S")

print (f'Device --> Host: {day}')
ser.write(day.encode(code))
ser.write(b'\n')
print (f'Device --> Host: {wct}')
ser.write(wct.encode(code))
ser.write(b'\n')

print ("Done sending. Answer:")

success = False
line = ser.readline()
while line:
    decoded_line = line.decode(code).strip()
    print (f'Host <-- Device: {decoded_line}')
    if "DS1307 configured!" in decoded_line:
        success = True
    line = ser.readline()

if success:
    print ("Got successful message, so assuming correctly set time.")
else:
    print ("Could not detect success message. Try resetting the device.")

print ("Trying to reset device via DTR")
ser.close()
ser.open()
ser.close()

# "Jun 19 2024"
# "19:26:52"