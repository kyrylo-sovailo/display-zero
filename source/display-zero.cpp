#include "../include/dz/display-zero.h"
#include <pigpio.h>
#include <stdexcept>

void dz::Display::_set_command_mode()
{
    gpioWrite(_mode_pin, 0);
}

void dz::Display::_set_data_mode()
{
    gpioWrite(_mode_pin, 1);
}

void dz::Display::command_swreset()
{
    char command = 0x01;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_slpin()
{
    char command = 0x10;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_slpout()
{
    char command = 0x11;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_invoff()
{
    char command = 0x20;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_invon()
{
    char command = 0x21;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_gamset(float gamma)
{
    char command = 0x26;
    char parameter;
    if (gamma == 1.0) parameter = 0x01;
    else if (gamma == 2.5) parameter = 0x02;
    else if (gamma == 2.2) parameter = 0x04;
    else if (gamma == 1.8) parameter = 0x08;
    else throw std::runtime_error("dz::Display::command_gamset(): invalid gamma");

    _set_command_mode();
    spiWrite(_device, &command, 1);
    _set_data_mode();
    spiWrite(_device, &parameter, 1);
}

void dz::Display::command_dispoff()
{
    char command = 0x28;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_dispon()
{
    char command = 0x29;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_caset(unsigned int begin, unsigned int end)
{
    char command = 0x2A;
    if (begin > 127 || end > 127 || begin > end) throw std::runtime_error("dz::Display::command_caset(): invalid araguments");
    char parameter[4];
    parameter[0] = (begin >> 8) & 0xFF;
    parameter[1] = begin & 0xFF;
    parameter[2] = (end >> 8) & 0xFF;
    parameter[3] = end & 0xFF;
    
    _set_command_mode();
    spiWrite(_device, &command, 1);
    _set_data_mode();
    spiWrite(_device, parameter, 4);
}

void dz::Display::command_raset(unsigned int begin, unsigned int end)
{
    char command = 0x2B;
    if (begin > 159 || end > 159 || begin > end) throw std::runtime_error("dz::Display::command_raset(): invalid araguments");
    char parameter[4];
    parameter[0] = (begin >> 8) & 0xFF;
    parameter[1] = begin & 0xFF;
    parameter[2] = (end >> 8) & 0xFF;
    parameter[3] = end & 0xFF;
    
    _set_command_mode();
    spiWrite(_device, &command, 1);
    _set_data_mode();
    spiWrite(_device, parameter, 4);
}

void dz::Display::command_ramwr(size_t length, char *data)
{
    char command = 0x2C;    
    _set_command_mode();
    spiWrite(_device, &command, 1);
    _set_data_mode();
    spiWrite(_device, data, length);
}

void dz::Display::command_rgbset(char r[32], char g[64], char b[32])
{
    char command = 0x2D;
    for (size_t i = 0; i < 32; i++)
    {
        if ((r[i] & 0xC0) != 0 || (g[i] & 0x80) != 0 || (b[i] & 0xC0) != 0) throw std::runtime_error("dz::Display::command_rgbset(): invalid araguments");
    }
    _set_command_mode();
    spiWrite(_device, &command, 1);
    _set_data_mode();
    spiWrite(_device, r, 32);
    spiWrite(_device, g, 64);
    spiWrite(_device, b, 32);
}

void dz::Display::command_teoff()
{
    char command = 0x34;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_teon()
{
    char command = 0x35;
    _set_command_mode();
    spiWrite(_device, &command, 1);
}

void dz::Display::command_madctl(bool row_inverse, bool column_inverse, bool row_column_swap, bool row_refresh_inverse, bool column_refresh_inverse, bool rgb_inverse)
{
    char command = 0x36;
    char parameter = 0;
    if (row_inverse) parameter |= (1 << 7);
    if (column_inverse) parameter |= (1 << 6);
    if (row_column_swap) parameter |= (1 << 5);
    if (row_refresh_inverse) parameter |= (1 << 4);
    if (rgb_inverse) parameter |= (1 << 3);
    if (column_refresh_inverse) parameter |= (1 << 2);
    _set_command_mode();
    spiWrite(_device, &command, 1);
    _set_data_mode();
    spiWrite(_device, &parameter, 1);
}

void dz::Display::command_colmod(unsigned int bits)
{
    char command = 0x3A;
    char parameter;
    if (bits == 12) parameter = 3;
    else if (bits == 16) parameter = 5;
    else if (bits == 18) parameter = 6;
    else throw std::runtime_error("dz::Display::command_colmod(): invalid bit number");

    _set_command_mode();
    spiWrite(_device, &command, 1);
    _set_data_mode();
    spiWrite(_device, &parameter, 1);
}

dz::Display::Display()
{
    //Initialization
    if (gpioInitialise() < 0) throw std::runtime_error("gpioInitialize failed");
    if (gpioWrite(_mode_pin, 0) < 0 || gpioWrite(_reset_pin, 0) < 0) throw std::runtime_error("dz::Display::Display(): gpioWrite failed");
    if (gpioSetMode(_mode_pin, PI_OUTPUT) < 0 || gpioSetMode(_reset_pin, PI_OUTPUT) < 0) throw std::runtime_error("dz::Display::Display(): gpioSetMode failed");
    if (gpioWrite(_mode_pin, 0) < 0 || gpioWrite(_reset_pin, 0) < 0) throw std::runtime_error("dz::Display::Display(): gpioWrite failed");
    _device = spiOpen(_spi_channel, 15*1000*1000, 0);
    if (_device < 0) throw std::runtime_error("spiOpen failed");

    //Hardware reset
    gpioWrite(_reset_pin, 0);
    gpioDelay(10*1000);
    gpioWrite(_reset_pin, 1);
    gpioDelay(120*1000);

    //Software reset
    command_swreset();
    gpioDelay(120*1000);

    //Display initialization
    command_slpout();
    command_madctl(false, false, false, false, false, false);
    command_colmod(18);
    command_gamset(1.0);
    command_invoff();
    command_teoff();
    command_dispon();
}

dz::Display::~Display()
{
    if (_device >= 0) spiClose(_device);
}