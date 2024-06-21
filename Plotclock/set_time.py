#/usr/bin/python

import serial
import datetime
import time
import sys

code = 'latin-1'

def send(serial, message, withNewline = False):
    print (f'Device --> Host: {message}')
    serial.write(message.encode(code))
    if withNewline:
        serial.write(b'\n')

def receive(serial):
    line = serial.readline()
    if line:
        decoded_line = line.decode(code).strip()
        if not decoded_line:
            decoded_line = "--whitespace--"
        print (f'Host <-- Device: {decoded_line}')
        return decoded_line



serial_device = '/dev/ttyUSB0'
if len(sys.argv) == 2:
    serial_device = sys.argv[1]
else:
    print (f"Usage: {sys.argv[0]} serial_device")

print (f'Using serial device "{serial_device}"')

ser = serial.Serial(serial_device, baudrate=115200, timeout=1, dsrdtr=True)
assert(ser.is_open)

print ("connected. Waiting for greeting text...")

tries = 10
remaining_tries = tries

line = receive(ser)
while line or remaining_tries != 0:
    if line:
        remaining_tries = tries
        if "Date like " in line:
            print ("Got expected greeting text!")
            break
    else:
        print (f"Waiting ({remaining_tries})...")
        idle_indicator = '*'
        send(ser, idle_indicator)
        remaining_tries -= 1
    line = receive(ser)

if remaining_tries == 0:
    print ("Timeout. Sending anyway!")

now = datetime.datetime.now(datetime.UTC)

day = now.strftime("%b %d %Y")
wct = now.strftime("%H:%M:%S")

send(ser, day, withNewline=True)
send(ser, wct, withNewline=True)
print ("Done sending. Answer:")

success = False
line = receive(ser)
while line:
    if "DS1307 configured!" in line:
        success = True
    line = receive(ser)

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