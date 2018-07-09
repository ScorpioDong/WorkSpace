# This is module for esp32 Speak 

import time
from machine import Pin, DAC

class Speak:
    def __init__(self):
        # init dac
        self.dac=DAC(Pin(25))

        # init params
        self.__volume = 8
        
     def play_music(self, data, rate):
         pass
