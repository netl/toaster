#!/usr/bin/python3
from periphery import SPI
import time

spi = SPI("/dev/spidev0.0", 0, 10000)
while 1:
   data=spi.transfer([0])
   print(data)
   time.sleep(0.1)
spi.close()
