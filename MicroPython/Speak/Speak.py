# This is module for esp32 Speak

import time
from machine import Pin, DAC


class Speak:
    def __init__(self):
        # init dac
        self.dac = DAC(Pin(25))

        # init params
        self.__volume = 8

    def set_volume(self, volume):
        if (volume <= 10) and (volume >= 0):
            self.__volume = 11 - volume
        else:
            print('the volume must be in 0 to 10')

    def play_music(self, filename, rate):
        delay_interval = int(1000000/rate)
        last_data = 0
        f = open(filename, 'r')
        if self.__volume != 11:
            while True:
                data = f.read(4)
                if data == '':
                    for i in range(int(last_data/self.__volume), 0, -1):
                        self.dac.write(i)
                        time.sleep_ms(2)
                    f.close()
                    return
                else:
                    data = int(data.strip())
                    self.dac.write(int(data/self.__volume))
                    time.sleep_us(delay_interval)
                    last_data = data
