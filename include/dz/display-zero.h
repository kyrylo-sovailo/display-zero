#pragma once

#include <stddef.h>

namespace dz
{
    class Display
    {
    private:
        static const unsigned int _mode_pin = 24;
        static const unsigned int _reset_pin = 25;
        static const unsigned int _spi_channel = 0;

        int _device = -1;

        void _set_command_mode();
        void _set_data_mode();

    public:
        static const unsigned int width = 128;
        static const unsigned int height = 160;

        void command_swreset();
        void command_slpin();
        void command_slpout();
        void command_invon();
        void command_invoff();
        void command_gamset(float gamma);
        void command_dispoff();
        void command_dispon();
        void command_caset(unsigned int begin, unsigned int end);
        void command_raset(unsigned int begin, unsigned int end);
        void command_ramwr(size_t length, char *data);
        void command_rgbset(char r[32], char g[64], char b[32]);
        void command_teoff();
        void command_teon();
        void command_madctl(bool row_inverse, bool column_inverse, bool row_column_swap, bool row_refresh_inverse, bool column_refresh_inverse, bool rgb_inverse);
        void command_colmod(unsigned int bits);

        Display();
        ~Display();
    };
}