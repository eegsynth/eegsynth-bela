#!/usr/bin/env python3

from gpiozero import LED
from time import sleep
import pause
from datetime import datetime,timedelta

red = LED(17)

now = datetime.now()
next = datetime(now.year, now.month, now.day, now.hour, now.minute, now.second, 0)

while True:
    next = next + timedelta(seconds=1)
    pause.until(next)
    red.on()
    sleep(0.2)
    red.off()
    sleep(0.2)
    
