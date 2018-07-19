

#include "modlcd.h"

lcd_dev_t lcd_dev = {
    .wramcmd = 0x2C,
    .dir = 0,
    .width = 128,
    .height = 128,
    .setxcmd = 0x2A,
    .setycmd = 0x2B,
};

static uint16_t GRAM[128 * 128] = {0};

static void delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

static void LCD_Writ_Bus(uint8_t da) //串行数据写入
{
    for (int i = 0; i < 8; i++)
    {
        if ((da << i) & 0x80)
            LCD_SDI_1;
        else
            LCD_SDI_0;

        LCD_SCK_0;
        LCD_SCK_1;
    }
}

//写16bit数据
static void LCD_Write_Bus16(uint16_t da) //串行数据写入
{
    for (int i = 0; i < 16; i++)
    {
        if ((da << i) & 0x8000)
            LCD_SDI_1;
        else
            LCD_SDI_0;

        LCD_SCK_0;
        LCD_SCK_1;
    }
}
static void LCD_WR_DATA(uint8_t da) //发送数据-8位参数
{
    LCD_CS_0;
    LCD_DC_1;
    LCD_Writ_Bus(da);
    LCD_CS_1;
}

//#define LCD_WR_DATA8 LCD_WR_DATA
static void LCD_WR_DATA16(uint16_t da)
{
    LCD_CS_0;
    LCD_DC_1;
    LCD_Write_Bus16(da);
    LCD_CS_1;
}

static void LCD_WR_REG(uint8_t da)
{
    LCD_CS_0;
    LCD_DC_0;
    LCD_Writ_Bus(da);
    LCD_CS_1;
}

// static void LCD_WR_REG_DATA(uint16_t reg, uint16_t da)
// {
//     LCD_WR_REG(reg);
//     LCD_WR_DATA(da);
// }

static void lcd_setpixel(uint16_t x, uint16_t y, uint16_t color)
{
    GRAM[x + y * lcd_dev.width] = color;
}

static uint16_t lcd_getpixel(uint16_t x, uint16_t y)
{
    return GRAM[x + y * lcd_dev.width];
}

static void lcd_set_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint16_t *b = &GRAM[x + y * lcd_dev.width];
    while (h--)
    {
        for (int ww = w; ww; --ww)
        {
            *b++ = color;
        }
        b += lcd_dev.width - w;
    }
}

static void lcd_setwindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd)
{
    LCD_WR_REG(lcd_dev.setxcmd);
    LCD_WR_DATA(xStar >> 8);
    LCD_WR_DATA((0x00FF & xStar) + 2);
    LCD_WR_DATA(xEnd >> 8);
    LCD_WR_DATA((0x00FF & xEnd) + 2);

    LCD_WR_REG(lcd_dev.setycmd);
    LCD_WR_DATA(yStar >> 8);
    LCD_WR_DATA((0x00FF & yStar) + 3);
    LCD_WR_DATA(yEnd >> 8);
    LCD_WR_DATA((0x00FF & yEnd) + 3);
    LCD_WR_REG(lcd_dev.wramcmd);
}

static void lcd_show()
{
    uint16_t i;
    lcd_setwindows(0, 0, lcd_dev.width - 1, lcd_dev.height - 1);
    for (i = 0; i < lcd_dev.width * lcd_dev.height; i++)
    {
        LCD_WR_DATA16(GRAM[i]);
    }
}

STATIC mp_obj_t lcd_fill(mp_obj_t c)
{
    uint16_t color = (uint16_t)mp_obj_get_int(c);
    lcd_set_fill_rect(0, 0, lcd_dev.width, lcd_dev.height, color);
    lcd_show();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lcd_fill_obj, lcd_fill);

STATIC mp_obj_t lcd_fill_rect(size_t n_args, const mp_obj_t *args)
{
    (void)n_args;

    uint16_t x = (uint16_t)mp_obj_get_int(args[0]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t w = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t h = (uint16_t)mp_obj_get_int(args[3]);
    uint16_t col = (uint16_t)mp_obj_get_int(args[4]);

    lcd_set_fill_rect(x, y, w, h, col);

    lcd_show();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_fill_rect_obj, 5, 5, lcd_fill_rect);

STATIC mp_obj_t lcd_pixel(size_t n_args, const mp_obj_t *args)
{
    // mp_obj_framebuf_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t x = (uint16_t)mp_obj_get_int(args[0]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[1]);
    if (0 <= x && x < lcd_dev.width && 0 <= y && y < lcd_dev.height)
    {
        if (n_args == 2)
        {
            // get
            return MP_OBJ_NEW_SMALL_INT(lcd_getpixel(x, y));
        }
        else
        {
            // set
            lcd_setpixel(x, y, (uint16_t)mp_obj_get_int(args[2]));
        }
    }
    lcd_show();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_pixel_obj, 2, 3, lcd_pixel);

STATIC mp_obj_t lcd_hline(size_t n_args, const mp_obj_t *args)
{
    (void)n_args;

    uint16_t x = (uint16_t)mp_obj_get_int(args[0]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t w = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t col = (uint16_t)mp_obj_get_int(args[3]);

    lcd_set_fill_rect(x, y, w, 1, col);

    lcd_show();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_hline_obj, 4, 4, lcd_hline);

STATIC mp_obj_t lcd_vline(size_t n_args, const mp_obj_t *args)
{
    (void)n_args;

    uint16_t x = (uint16_t)mp_obj_get_int(args[0]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t h = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t col = (uint16_t)mp_obj_get_int(args[3]);

    lcd_set_fill_rect(x, y, 1, h, col);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_vline_obj, 4, 4, lcd_vline);

STATIC mp_obj_t lcd_rect(size_t n_args, const mp_obj_t *args) {
    (void)n_args;

   uint16_t x = (uint16_t)mp_obj_get_int(args[0]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t w = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t h = (uint16_t)mp_obj_get_int(args[3]);
    uint16_t col = (uint16_t)mp_obj_get_int(args[4]);

    lcd_set_fill_rect(x, y, w, 1, col);
    lcd_set_fill_rect(x, y + h- 1, w, 1, col);
    lcd_set_fill_rect(x, y, 1, h, col);
    lcd_set_fill_rect(x + w- 1, y, 1, h, col);

    lcd_show();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_rect_obj, 5, 5,lcd_rect);

STATIC mp_obj_t lcd_line(size_t n_args, const mp_obj_t *args) {
    (void)n_args;

     uint16_t x1 = (uint16_t)mp_obj_get_int(args[0]);
    uint16_t y1 = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t x2 = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t y2 = (uint16_t)mp_obj_get_int(args[3]);
    uint16_t col = (uint16_t)mp_obj_get_int(args[4]);

    uint16_t dx = x2 - x1;
    uint16_t sx;
    if (dx > 0) {
        sx = 1;
    } else {
        dx = -dx;
        sx = -1;
    }

    uint16_t dy = y2 - y1;
    uint16_t sy;
    if (dy > 0) {
        sy = 1;
    } else {
        dy = -dy;
        sy = -1;
    }

    bool steep;
    if (dy > dx) {
        uint16_t temp;
        temp = x1; x1 = y1; y1 = temp;
        temp = dx; dx = dy; dy = temp;
        temp = sx; sx = sy; sy = temp;
        steep = true;
    } else {
        steep = false;
    }

    uint16_t e = 2 * dy - dx;
    for (uint16_t i = 0; i < dx; ++i) {
        if (steep) {
            if (0 <= y1 && y1 < lcd_dev.width && 0 <= x1 && x1 < lcd_dev.height) {
                lcd_setpixel(y1, x1, col);
            }
        } else {
            if (0 <= x1 && x1 < lcd_dev.width && 0 <= y1 && y1 < lcd_dev.height) {
                lcd_setpixel(x1, y1, col);
            }
        }
        while (e >= 0) {
            y1 += sy;
            e -= 2 * dx;
        }
        x1 += sx;
        e += 2 * dy;
    }

    if (0 <= x2 && x2 < lcd_dev.width && 0 <= y2 && y2 < lcd_dev.height) {
        lcd_setpixel(x2, y2, col);
    }

    lcd_show();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_line_obj, 5, 5, lcd_line);

STATIC mp_obj_t lcd_scroll(mp_obj_t xstep_in, mp_obj_t ystep_in) {
    uint16_t xstep = (uint16_t)mp_obj_get_int(xstep_in);
    uint16_t ystep = (uint16_t)mp_obj_get_int(ystep_in);
    uint16_t sx, y, xend, yend, dx, dy;
    if (xstep < 0) {
        sx = 0;
        xend = lcd_dev.width + xstep;
        dx = 1;
    } else {
        sx = lcd_dev.width - 1;
        xend = xstep - 1;
        dx = -1;
    }
    if (ystep < 0) {
        y = 0;
        yend = lcd_dev.height + ystep;
        dy = 1;
    } else {
        y = lcd_dev.height - 1;
        yend = ystep - 1;
        dy = -1;
    }
    for (; y != yend; y += dy) {
        for (int x = sx; x != xend; x += dx) {
            lcd_setpixel(x, y, lcd_getpixel(x - xstep, y - ystep));
        }
    }

    lcd_show();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(lcd_scroll_obj, lcd_scroll);

STATIC mp_obj_t lcd_text(size_t n_args, const mp_obj_t *args) {

    const char *str = mp_obj_str_get_str(args[0]);
    uint16_t x0 = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t y0 = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t col = 1;
    if (n_args >= 4) {
        col = (uint16_t)mp_obj_get_int(args[3]);
    }

    // loop over chars
    for (; *str; ++str) {
        // get char and make sure its in range of font
        int chr = *(uint8_t*)str;
        if (chr < 32 || chr > 127) {
            chr = 127;
        }
        // get char data
        const uint8_t *chr_data = &font_petme128_8x8[(chr - 32) * 8];
        // loop over char data
        for (int j = 0; j < 8; j++, x0++) {
            if (0 <= x0 && x0 < lcd_dev.width) { // clip x
                uint vline_data = chr_data[j]; // each byte is a column of 8 pixels, LSB at top
                for (int y = y0; vline_data; vline_data >>= 1, y++) { // scan over vertical column
                    if (vline_data & 1) { // only draw if pixel set
                        if (0 <= y && y < lcd_dev.height) { // clip y
                            lcd_setpixel(x0, y, col);
                        }
                    }
                }
            }
        }
    }

    lcd_show();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lcd_text_obj, 3, 4, lcd_text);

static void lcd_gpio_init()
{

    gpio_pad_select_gpio(MOSI_PIN);
    gpio_pad_select_gpio(SCK_PIN);
    gpio_pad_select_gpio(CS_PIN);
    gpio_pad_select_gpio(RS_PIN);
    gpio_pad_select_gpio(RST_PIN);
    gpio_pad_select_gpio(LED_PIN);

    gpio_set_level(MOSI_PIN, 1);
    gpio_set_level(SCK_PIN, 1);
    gpio_set_level(CS_PIN, 0);
    gpio_set_level(RS_PIN, 1);
    gpio_set_level(RST_PIN, 1);
    gpio_set_level(LED_PIN, 0);

    //gpio_set_direction(MISO_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(MOSI_PIN, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(SCK_PIN, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(CS_PIN, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(RS_PIN, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(RST_PIN, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(LED_PIN, GPIO_MODE_DEF_OUTPUT);
}

STATIC mp_obj_t lcd_init()
{
    lcd_gpio_init();
    //复位
    LCD_REST_0;
    delay(100);
    LCD_REST_1;
    delay(50);

    LCD_WR_REG(0x11); //Sleep exit
    delay(120);

    LCD_WR_REG(0x13); //Sleep exit
    //ST7735R Frame Rate
    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x2D);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x2D);

    LCD_WR_REG(0xB3);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x2D);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x2D);

    LCD_WR_REG(0xB4); //Column inversion
    LCD_WR_DATA(0x07);

    //ST7735R Power Sequence
    LCD_WR_REG(0xC0);
    LCD_WR_DATA(0xA2);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x84);
    LCD_WR_REG(0xC1);
    LCD_WR_DATA(0xC5);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA(0x8A);
    LCD_WR_DATA(0x2A);
    LCD_WR_REG(0xC4);
    LCD_WR_DATA(0x8A);
    LCD_WR_DATA(0xEE);

    LCD_WR_REG(0xC5); //VCOM
    LCD_WR_DATA(0x0E);

    LCD_WR_REG(0x36); //MX, MY, RGB mode
    LCD_WR_DATA(0xC8);

    //ST7735R Gamma Sequence
    LCD_WR_REG(0xe0);
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x1a);
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x2f);
    LCD_WR_DATA(0x28);
    LCD_WR_DATA(0x20);
    LCD_WR_DATA(0x22);
    LCD_WR_DATA(0x1f);
    LCD_WR_DATA(0x1b);
    LCD_WR_DATA(0x23);
    LCD_WR_DATA(0x37);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x10);

    LCD_WR_REG(0xe1);
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x1b);
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x2c);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x2e);
    LCD_WR_DATA(0x30);
    LCD_WR_DATA(0x30);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x3f);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x10);

    LCD_WR_REG(0x2a);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x7f);

    LCD_WR_REG(0x2b);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x9f);

    LCD_WR_REG(0xF0); //Enable test command
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xF6); //Disable ram power save mode
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x3A); //65k mode
    LCD_WR_DATA(0x05);
    LCD_WR_REG(0x29); //Display on

    for (int i = 0; i < 128 * 128; i++)
    {
        GRAM[i] = 0xFFFF;
    }

    lcd_show();

    LCD_BL_1;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(lcd_init_obj, lcd_init);

STATIC const mp_rom_map_elem_t lcd_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lcd)},

    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lcd_init_obj)},
    {MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&lcd_fill_obj)},
    {MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&lcd_pixel_obj)},
    {MP_ROM_QSTR(MP_QSTR_fill_rect), MP_ROM_PTR(&lcd_fill_rect_obj)},
    {MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&lcd_line_obj)},
    {MP_ROM_QSTR(MP_QSTR_hline), MP_ROM_PTR(&lcd_hline_obj)},
    {MP_ROM_QSTR(MP_QSTR_vline), MP_ROM_PTR(&lcd_vline_obj)},
    {MP_ROM_QSTR(MP_QSTR_scroll), MP_ROM_PTR(&lcd_scroll_obj)},
    {MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&lcd_text_obj)},
};

STATIC MP_DEFINE_CONST_DICT(lcd_module_globals, lcd_module_globals_table);

const mp_obj_module_t lcd_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&lcd_module_globals,
};