#!/usr/bin/env python3

import os
from gpiozero import LED, Button
from time import sleep
import pause
from datetime import datetime,timedelta

red = LED(17)
button = Button(18, pull_up=False)

def blink():
    red.on()
    sleep(0.2)
    red.off()

def sync():
  global usesync, lastsync
  usesync = True
  lastsync = datetime.now()
  print("sync")
  blink()

button.when_pressed = sync

usesync = False
lastsync = datetime.now() - timedelta(seconds=60)

while True:
    now = datetime.now()
    next = datetime(now.year, now.month, now.day, now.hour, now.minute, now.second, 0) + timedelta(seconds=1)
    pause.until(next)

    if not usesync:
      print("clock")
      blink()
    else:
      if now - lastsync > timedelta(seconds=5):
        usesync = False

