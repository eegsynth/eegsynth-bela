# synchronize-ttl

This is not an EEGsynth project per see, but relates to it.

With the code in this project I am exploring how accurately one can
synchronize multiple Raspberry Pi's. The TTL level output of GPIO pin
17 is connected over a 330 Ohm resistor to a LED. The python code on
the Raspberries causes them to blink more or less synchronous, using
the system clock.

The TTL output (3.3V) of the Raspberries is also connected to the analog
input channels of the Bela (which is max 4V) and displayed on the scope. I am
using the scope to trigger on the rising flank of one of the TTL signals,
and can look at the relative time of the others.

## References

- [Connecting a LED to the Raspberry Pi](https://thepihut.com/blogs/raspberry-pi-tutorials/27968772-turning-on-an-led-with-your-raspberry-pis-gpio-pins)
- [Gpiozero](https://gpiozero.readthedocs.io/)
- [CircuitPython](https://circuitpython.readthedocs.io/)

