#!/usr/bin/env python3

import os
from gpiozero import LED, Button
from time import sleep
import pause
from datetime import datetime,timedelta

red = LED(17)
button = Button(18, pull_up=False)

def upon_sync():
    now = datetime.now()
    print("%d.%06d" % (now.second, now.microsecond))
    if now.microsecond < 500000:
       os.system("sudo /home/pi/adjtime %d %d" % (0, -now.microsecond))
    else:
       os.system("sudo /home/pi/adjtime %d %d" % (0, 1000000-now.microsecond))

button.when_pressed = upon_sync

now = datetime.now()
next = datetime(now.year, now.month, now.day, now.hour, now.minute, now.second, 0)
next = next + timedelta(seconds=1)

while True:
    pause.until(next)
    red.on()
    sleep(0.2)
    red.off()
    sleep(0.2)
    next = next + timedelta(seconds=1)

