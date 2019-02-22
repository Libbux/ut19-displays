# simulator.py
# By Jonathan Libby, February 2019
# This is a very simple script that simulates the serial traffic sent by the
# master device in the setup described in this repository's README.md, in order
# to aid the development process for the slave devices which drive the in-car
# displays.

# PySerial - used to read/write to Serial devices
import serial
from time import sleep
import random

# Serial connection parameters (these are hardware dependent)
baud_rate = 2000000

# Open a serial connection to the device (usually via USB->serial bridge)
# (It's easiest to get the device name from the Arduino IDE serial monitor)
port = serial.Serial('/dev/cu.usbserial-A906HARW', baud_rate, timeout=50)
print("Connected over serial port: " + port.name)

# Send simulated messages over serial
rpm = 0
batt = 14.3
temp = 75

# Start by displaying startup
port.write(str(batt) + "R" + str(rpm) +"C" + str(temp) + "T")
sleep(4)

# Display 50 screens
for i in range(0, 50):
    # Then, increment RPM and update display along the way
    for j in range(0, 10):
        if (rpm >= 10000 or rpm <= 2000):
            rpm = 4500
        else:
            # Biased towards revving upwards to mimic actual driving
            rpm += int(random.uniform(-600, 1000))
        # Write to device
        port.write(str(batt) + "R" + str(rpm) +"C" + str(temp) + "T")
        # Flush the buffer so that we can read from serial immediately afterwards
        port.flush()
        # Slow this down so the device isn't flooded
        sleep(0.1)

    # Update other values
    if (batt < 11):
        batt = 14.3
    else:
        batt -= round(random.uniform(0.2, 0.7), 1)

    if (temp >= 110):
        temp = 80
    else:
        temp += int(random.uniform(2, 6))

line = port.read()
print(line)

# End by displaying shutdown
port.write(str(batt) + "R" + str(0) +"C" + str(temp) + "T")

# Close serial connection
port.close()
