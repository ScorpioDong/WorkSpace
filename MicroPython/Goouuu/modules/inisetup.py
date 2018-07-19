import uos
from flashbdev import bdev

def check_bootsec():
    buf = bytearray(bdev.SEC_SIZE)
    bdev.readblocks(0, buf)
    empty = True
    for b in buf:
        if b != 0xff:
            empty = False
            break
    if empty:
        return True
    fs_corrupted()

def fs_corrupted():
    import time
    while 1:
        print("""\
FAT filesystem appears to be corrupted. If you had important data there, you
may want to make a flash snapshot to try to recover it. Otherwise, perform
factory reprogramming of MicroPython firmware (completely erase flash, followed
by firmware programming).
""")
        time.sleep(3)

def setup():
    check_bootsec()
    print("Performing initial setup")
    uos.VfsFat.mkfs(bdev)
    vfs = uos.VfsFat(bdev)
    uos.mount(vfs, '/flash')
    uos.chdir('/flash')
    with open("boot.py", "w") as f:
        f.write("""\
# This file is executed on every boot (including wake-boot from deepsleep)
# import sdcard
# import os
# from machine import Pin,SPI
# sd = sdcard.SDCard(SPI(sck=Pin(18),mosi=Pin(23),miso=Pin(19)),Pin(5))
# os.mount(sd,'/sd')
# os.chdir('/sd')
""")
    return vfs
