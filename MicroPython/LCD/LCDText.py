import LCD
from machine import Pin,SPI


def main():
    spi = SPI(sck=Pin(18), mosi=Pin(23), miso=Pin(19))
    lcd = LCD.LCD(spi, cs=Pin(26), rs=Pin(17), rst=Pin(16), led=Pin(13))
    lcd.lcd_clear(lcd.color_white)
    lcd.set_back_color(lcd.color_blue)
    lcd.set_point_color(lcd.color_red)
    lcd.lcd_show_string(0,0,12,'Hello MicroPython',1)
