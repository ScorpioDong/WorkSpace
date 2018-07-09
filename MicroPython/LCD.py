# MicroPython driver for ESP32 LCD using ESP32 SPI bus

import time
import font


class LCD:
    def __init__(self, spi, cs, rs, rst, led):
        self.spi = spi
        self.cs = cs
        self.rs = rs
        self.rst = rst
        self.led = led

        # init Params
        self.width = 0
        self.height = 0
        self.id = 0
        self.dir = 0
        self.wramcmd = 0
        self.setxcmd = 0
        self.setycmd = 0

        # 颜色定义
        self.color_white = 0xffff
        self.color_black = 0x0000
        self.color_blue = 0x001f
        self.color_bred = 0xf81f
        self.color_gred = 0xffe0
        self.color_gblue = 0x07ff
        self.color_red = 0xf800
        self.color_magenta = 0xf81f
        self.color_green = 0x07e0
        self.color_cyan = 0x7fff
        self.color_yellow = 0xffe0
        self.color_brown = 0xbc40
        self.color_brred = 0xfc07
        self.color_gray = 0x8430

        # 画笔颜色 背景颜色
        self.point_color = self.color_black
        self.back_color = self.color_white

        self.data_buf = bytearray(2)

        self.init_lcd()

    def init_lcd(self):
        # init Pin
        self.rs.init(self.rs.OUT, value=1)
        self.cs.init(self.cs.OUT, value=1)
        self.rst.init(self.rst.OUT, value=1)
        self.led.init(self.led.OUT, value=0)

        # init SPI bus
        self.init_spi(10 * 1024 * 1024)

        # lcd_params init
        self.lcd_reset()

        self.lcd_wr_reg(0x11)
        time.sleep_ms(120)

        self.lcd_wr_reg(0x13)

        self.lcd_wr_reg(0xb1)
        self.lcd_wr_data(0x01)
        self.lcd_wr_data(0x2c)
        self.lcd_wr_data(0x2d)

        self.lcd_wr_reg(0xb2)
        self.lcd_wr_data(0x01)
        self.lcd_wr_data(0x2c)
        self.lcd_wr_data(0x2d)

        self.lcd_wr_reg(0xb3)
        self.lcd_wr_data(0x01)
        self.lcd_wr_data(0x2c)
        self.lcd_wr_data(0x2d)
        self.lcd_wr_data(0x01)
        self.lcd_wr_data(0x2c)
        self.lcd_wr_data(0x2d)

        self.lcd_wr_reg(0xb4)
        self.lcd_wr_data(0x07)

        self.lcd_wr_reg(0xc0)
        self.lcd_wr_data(0xa2)
        self.lcd_wr_data(0x02)
        self.lcd_wr_data(0x84)
        self.lcd_wr_reg(0xc1)
        self.lcd_wr_data(0xc5)

        self.lcd_wr_reg(0xc2)
        self.lcd_wr_data(0x0a)
        self.lcd_wr_data(0x20)

        self.lcd_wr_reg(0xc3)
        self.lcd_wr_data(0x8a)
        self.lcd_wr_data(0x2a)
        self.lcd_wr_reg(0xc4)
        self.lcd_wr_data(0x8a)
        self.lcd_wr_data(0xee)

        self.lcd_wr_reg(0xc5)
        self.lcd_wr_data(0x0e)

        self.lcd_wr_reg(0x36)
        self.lcd_wr_data(0xc8)

        self.lcd_wr_reg(0xe0)
        self.lcd_wr_data(0x0f)
        self.lcd_wr_data(0x1a)
        self.lcd_wr_data(0x0f)
        self.lcd_wr_data(0x18)
        self.lcd_wr_data(0x2f)
        self.lcd_wr_data(0x28)
        self.lcd_wr_data(0x20)
        self.lcd_wr_data(0x22)
        self.lcd_wr_data(0x1f)
        self.lcd_wr_data(0x1b)
        self.lcd_wr_data(0x23)
        self.lcd_wr_data(0x37)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x07)
        self.lcd_wr_data(0x02)
        self.lcd_wr_data(0x10)

        self.lcd_wr_reg(0xe1)
        self.lcd_wr_data(0x0f)
        self.lcd_wr_data(0x1b)
        self.lcd_wr_data(0x0f)
        self.lcd_wr_data(0x17)
        self.lcd_wr_data(0x33)
        self.lcd_wr_data(0x2c)
        self.lcd_wr_data(0x29)
        self.lcd_wr_data(0x2e)
        self.lcd_wr_data(0x30)
        self.lcd_wr_data(0x30)
        self.lcd_wr_data(0x39)
        self.lcd_wr_data(0x3f)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x07)
        self.lcd_wr_data(0x03)
        self.lcd_wr_data(0x10)

        self.lcd_wr_reg(0x2a)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x7f)

        self.lcd_wr_reg(0x2b)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x00)
        self.lcd_wr_data(0x9f)

        self.lcd_wr_reg(0xf0)
        self.lcd_wr_data(0x01)
        self.lcd_wr_reg(0xf6)
        self.lcd_wr_data(0x00)

        self.lcd_wr_reg(0xf3a)
        self.lcd_wr_data(0x05)
        self.lcd_wr_reg(0x29)

        self.lcd_set_param()
        self.led.value(1)

    def init_spi(self, baudrate):
        try:
            master = self.spi.MASTER
        except AttributeError:
            self.spi.init(baudrate=baudrate, phase=0, polarity=0)

    def lcd_wr_reg(self, data):

        self.cs.value(0)
        self.rs.value(0)

        self.spi.write(bytearray([data]))
        self.cs.value(1)

    def lcd_wr_data(self, data):

        self.cs.value(0)
        self.rs.value(1)

        self.spi.write(bytearray([data]))
        self.cs.value(1)

    def lcd_wr_data_16bit(self, data):

        self.cs.value(0)
        self.rs.value(1)

        self.spi.write(bytearray([data >> 8]))
        self.spi.write(bytearray([data]))
        self.cs.value(1)

    def lcd_write_reg(self, reg, regvalue):
        self.lcd_wr_reg(reg)
        self.lcd_wr_data(regvalue)

    def lcd_write_ram_prepare(self):
        self.lcd_wr_reg(self.wramcmd)

    def lcd_set_cursor(self, xpos, ypos):
        self.lcd_set_windows(xpos, ypos, xpos, ypos)

    def lcd_set_param(self):
        self.wramcmd = 0x2C
        self.dir = 0
        self.width = 128
        self.height = 128
        self.setxcmd = 0x2a
        self.setycmd = 0x2b

        self.lcd_write_reg(0x36, 0xc8)

    def lcd_draw_point(self, x, y):
        self.lcd_set_cursor(x, y)
        self.lcd_wr_data_16bit(self.point_color)

    def lcd_reset(self):
        self.rst.value(0)
        time.sleep_ms(100)
        self.rst.value(1)
        time.sleep_ms(50)

    """
    全屏填充
    """
    def lcd_clear(self, color):
        self.lcd_set_windows(0, 0, self.width-1, self.height-1)
        for i in range(self.width):
            for j in range(self.height):
                self.lcd_wr_data_16bit(color)

    def lcd_set_windows(self, xstar, ystar, xend, yend):
        self.lcd_wr_reg(self.setxcmd)
        self.lcd_wr_data(int(xstar) >> 8)
        self.lcd_wr_data(0x00ff & int(xstar) + 2)
        self.lcd_wr_data(int(xend) >> 8)
        self.lcd_wr_data(0x00ff & int(xend) + 2)

        self.lcd_wr_reg(self.setycmd)
        self.lcd_wr_data(int(ystar) >> 8)
        self.lcd_wr_data(0x00ff & int(ystar) + 2)
        self.lcd_wr_data(int(yend) >> 8)
        self.lcd_wr_data(0x00ff & int(yend) + 2)

        self.lcd_write_ram_prepare()

    """
    画点
    """
    def gui_draw_point(self, x, y, color):
        self.lcd_set_cursor(x, y)
        self.lcd_wr_data_16bit(color)

    """
    指定区域填充
    """
    def lcd_fill(self, sx, sy, ex, ey, color):
        n_width = ex-sx+1
        n_height = ey-sy+1
        self.lcd_set_windows(sx, sy, ex-1, ey-1)
        for i in range(n_height):
            for j in range(n_width):
                self.lcd_wr_data_16bit(color)

        self.lcd_set_windows(0, 0, self.width-1, self.height-1)

    """
    画线
    """
    def lcd_draw_line(self, x1, y1, x2, y2):
        delta_x = x2-x1
        delta_y = y2-y1
        urow = x1
        urol = y1
        xerr = 0
        yerr = 0

        if delta_x > 0:
            incx = 1
        elif delta_x == 0:
            incx = 0
        else:
            incx = -1
            delta_x = -delta_x

        if delta_y > 0:
            incy = 1
        elif delta_y == 0:
            incy = 0
        else:
            incy = -1
            delta_y = -delta_y

        if delta_x > delta_y:
            distance = delta_x
        else:
            distance = delta_y

        for t in range(distance+1):
            self.lcd_draw_point(urow, urol)
            xerr += delta_x
            yerr += delta_y
            if xerr > distance:
                xerr -= distance
                urow += incx
            if yerr > distance:
                yerr -= distance
                urol += incy

    """
    矩形不填充
    """
    def lcd_draw_rectangle(self, x1, y1, x2, y2):
        self.lcd_draw_line(x1, y1, x2, y1)
        self.lcd_draw_line(x1, y1, x1, y2)
        self.lcd_draw_line(x1, y2, x2, y2)
        self.lcd_draw_line(x2, y1, x2, y2)

    """
    矩形填充
    """
    def lcd_draw_fill_rectangle(self, x1, y1, x2, y2):
        self.lcd_fill(x1, y1, x2, y2, self.point_color)

    def __draw_circle_8(self, xc, yc, x, y, c):
        self.gui_draw_point(xc + x, yc + y, c)
        self.gui_draw_point(xc - x, yc + y, c)
        self.gui_draw_point(xc + x, yc - y, c)
        self.gui_draw_point(xc - x, yc - y, c)
        self.gui_draw_point(xc + x, yc + y, c)
        self.gui_draw_point(xc - x, yc + y, c)
        self.gui_draw_point(xc + x, yc - y, c)
        self.gui_draw_point(xc - x, yc - y, c)

    """
    画圆
        xc,yc 圆心坐标
        c 颜色
        r 半径
        fill 0 不填充 1 填充
    """
    def gui_circle(self, xc, yc, c, r, fill):
        x = 0
        y = r
        d = 3 - 2 * r
        if fill:
            while x <= y:
                for yi in range(x, y+1):
                    self.__draw_circle_8(xc, yc, x, yi, c)

                if d < 0:
                    d = d + 4 * x + 6
                else:
                    d = d + 4 * (x - y) + 10
                    y -= 1
                x += 1
        else:
            while x <= y:
                self.__draw_circle_8(xc, yc, x, y, c)
                if d < 0:
                    d = d + 4 * x + 6
                else:
                    d = d + 4 * (x - y) + 10
                    y -= 1
                x += 1

    """
    显示字符
        x，y 起点坐标
        length 字体大小
        char 字符
        mod 0 填充模式 1 叠加模式
    """
    def lcd_show_char(self, x, y, fc, bc, char, length, mod):
        color_temp = self.point_color
        num = ord(char) - ord(' ')
        self.lcd_set_windows(x, y, x+length/2-1, y+length-1)
        if not mod:
            for pos in range(length):
                if length == 12:
                    temp = font.asc2_1206[num][pos]
                else:
                    temp = font.asc2_1608[num][pos]
                for t in range(length/2):
                    if temp & 0x01:
                        self.lcd_wr_data_16bit(fc)
                    else:
                        self.lcd_wr_data_16bit(bc)
                    temp >>= 1
        else:
            for pos in range(length):
                if length == 12:
                    temp = font.asc2_1206[num][pos]
                else:
                    temp = font.asc2_1608[num][pos]
                for t in range(length/2):
                    self.point_color = fc
                    if temp & 0x01:
                        self.lcd_draw_point(x+t, y+pos)
                    temp >>= 1
        self.point_color = color_temp
        self.lcd_set_windows(0, 0, self.width-1, self.height-1)

    """
    显示字符串
        x，y 起点坐标
        length 字体大小
        string 字符串
        mod 0 填充模式 1 叠加模式
    """
    def lcd_show_string(self, x, y, length, string, mod):
        num = len(string)
        for i in range(num):
            if (ord(string[i]) <= ord('~')) and (ord(string[i]) >= ord(' ')):
                if (x > self.width-1) or (y > self.height-1):
                    return
                self.lcd_show_char(x, y, self.point_color, self.back_color, string[i], length, mod)
                x += length/2

    def get_point_color(self):
        return self.point_color

    def set_point_color(self, color):
        self.point_color = color

    def get_back_color(self):
        return self.back_color

    def set_back_color(self, color):
        self.back_color = color
