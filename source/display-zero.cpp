#include "../include/dz/display-zero.h"
#include <libavcodec/avcodec.h>
#include <FreeImage.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>

#ifdef LIBDISPLAY_ZERO

int display(const char *input, unsigned int period)
{
    //Load file
    std::ifstream stream(input, std::ios::binary);
    if (!stream.good()) throw std::runtime_error("process(): opening input file failed");
    stream.seekg(0, std::ios::end);
    unsigned int size = stream.tellg();
    stream.seekg(0, std::ios::beg);
    if (size % (dz::Display::width*dz::Display::height*3) != 0) throw std::runtime_error("process(): corrupted input file");
    std::vector<char> display_raw(size);
    if (!stream.read(&display_raw[0], display_raw.size())) throw std::runtime_error("process(): reading from file failed");
    unsigned int nframes = size / (dz::Display::width*dz::Display::height*3);
    
    //Initialize timer
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    //Display
    dz::Display display;
    while (true)
    {
        for (unsigned int i = 0; i < nframes; i++)
        {
            //Write display
            display.command_caset(0, dz::Display::width-1);
            display.command_raset(0, dz::Display::height-1);
            display.command_ramwr(dz::Display::height*dz::Display::width*3, &display_raw[dz::Display::width*dz::Display::height*3*i]);
            
            //Wait
            time.tv_nsec += 1000*1000*period;
            if (time.tv_nsec > 1000*1000*1000) { time.tv_nsec -= 1000*1000*1000; time.tv_sec += 1; }
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, nullptr);
        }
    }
    return 0;
}

#endif

int process(const char *input, const char *output)
{
    //Open GIF
    FIMULTIBITMAP *animation_bitmap = FreeImage_OpenMultiBitmap(FIF_GIF, input, false, true, true, GIF_PLAYBACK);
    if (animation_bitmap == nullptr) throw std::runtime_error("process(): FreeImage_OpenMultiBitmap failed");
    unsigned int nframes = FreeImage_GetPageCount(animation_bitmap);
    std::vector<char> display_raw(nframes*dz::Display::width*dz::Display::height*3);
    std::vector<unsigned char> frame_raw;
    for (unsigned int frame = 0; frame < nframes; frame++)
    {
        //Open frame
        std::cout << "Processing frame " << frame+1 << "/" << nframes << std::endl;
        FIBITMAP *frame_bitmap = FreeImage_LockPage(animation_bitmap, frame);
        if (frame_bitmap == nullptr) { FreeImage_CloseMultiBitmap(animation_bitmap, 0); throw std::runtime_error("process(): FreeImage_LockPage failed"); }
        unsigned int frame_height = FreeImage_GetHeight(frame_bitmap);
        unsigned int frame_width = FreeImage_GetWidth(frame_bitmap);
        unsigned int frame_pitch = FreeImage_GetPitch(frame_bitmap);
        frame_raw.resize(frame_height * frame_pitch);
        FreeImage_ConvertToRawBits(&frame_raw[0], frame_bitmap, frame_pitch, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, true);

        //Resize
        const unsigned int window_top = 0;
        const unsigned int window_left = 33;
        const unsigned int window_height = 300;
        const unsigned int window_width = 375;
        for (unsigned int display_top = 0; display_top < dz::Display::height; display_top++)
        {
            float pixel_left = (0.5f + (float)window_left + (float)display_top * (window_width - 1) / (dz::Display::height - 1)) - 0.5f;
            unsigned int pixel_left0 = (unsigned int)floor(pixel_left);
            unsigned int pixel_left1 = ((pixel_left0 + 1) < frame_width) ? (pixel_left0 + 1) : (pixel_left0);
            for (unsigned int display_left = 0; display_left < dz::Display::width; display_left++)
            {
                float pixel_top = (0.5f + (float)window_top + (float)display_left * (window_height - 1) / (dz::Display::width - 1)) - 0.5f;
                unsigned int pixel_top0 = (unsigned int)floor(pixel_top);
                unsigned int pixel_top1 = ((pixel_top0 + 1) < frame_height) ? (pixel_top0 + 1) : (pixel_top0);
                for (unsigned int c = 0; c < 3; c++)
                {
                    display_raw[dz::Display::height*dz::Display::width*3*frame + dz::Display::width*3*display_top + 3*display_left + c] = ((unsigned char)((
                        frame_raw[frame_pitch*pixel_top0 + 4*pixel_left0 + 2 - c] * (1.0f - (pixel_top - pixel_top0)) * (1.0f - (pixel_left - pixel_left0)) +
                        frame_raw[frame_pitch*pixel_top0 + 4*pixel_left1 + 2 - c] * (1.0f - (pixel_top - pixel_top0)) * (pixel_left - pixel_left0) +
                        frame_raw[frame_pitch*pixel_top1 + 4*pixel_left0 + 2 - c] * (pixel_top - pixel_top0) * (1.0f - (pixel_left - pixel_left0)) +
                        frame_raw[frame_pitch*pixel_top1 + 4*pixel_left1 + 2 - c] * (pixel_top - pixel_top0) * (pixel_left - pixel_left0)
                    ) * 63.0f / 255.0f)) << 2;
                }
            }
        }
        FreeImage_UnlockPage(animation_bitmap, frame_bitmap, true);
    }
    FreeImage_CloseMultiBitmap(animation_bitmap, 0);

    //Write file
    std::ofstream stream(output, std::ios::binary);
    if (!stream.good()) throw std::runtime_error("process(): opening output file failed");
    if (!stream.write(&display_raw[0], display_raw.size())) throw std::runtime_error("process(): writing to file file failed");

    return 0;
}

int help()
{
    std::cout << "display-zero is an utility that displays images and videos using display-zero library" << std::endl;
    std::cout << "Usage:" << std::endl;
    #ifdef LIBDISPLAY_ZERO
    std::cout << "    ./display-zero <OUTPUT> -t <PERIOD>   - display processed file file with given period (if video)" << std::endl;
    #endif
    std::cout << "    ./display-zero -p <INPUT> -o <OUTPUT> - process file" << std::endl;
    std::cout << "    ./display-zero -h                     - print help" << std::endl;
    return 0;
}

int _main(int argc, char **argv)
{
    try
    {
        const char *operation = nullptr;
        const char *input = nullptr;
        const char *output = nullptr;
        unsigned int period = 0;
        bool next_output = false;
        bool next_period = false;
        for (unsigned int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
                { if (output != nullptr) throw std::runtime_error("_main(): repeated -o option"); next_output = true; }
	        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--time") == 0)
                { if (period > 0) throw std::runtime_error("_main(): repeated -t option"); next_period = true; }
            else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--process") == 0)
                { if (operation != nullptr) throw std::runtime_error("_main(): repeated operation option"); operation = "-p"; }
            else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
                { if (operation != nullptr) throw std::runtime_error("_main(): repeated operation option"); operation = "-h"; }
            else if (next_output)
                { output = argv[i]; next_output = false; }
            else if (next_period)
                { char *end; period = strtoul(argv[i], &end, 10); if (*end != '\0' || period == 0) throw std::runtime_error("_main(): invalid -t option"); next_period = false; }
            else
                { input = argv[i]; }
        }

        #ifdef LIBDISPLAY_ZERO
            if (operation == nullptr && input != nullptr && output == nullptr && period != 0 && !next_output && !next_period) return display(input, period);
            else
        #endif
        if (operation != nullptr && strcmp(operation, "-p") == 0 && input != nullptr && output != nullptr && period == 0 && !next_output && !next_period) return process(input, output);
        else if (operation != nullptr && strcmp(operation, "-h") == 0 && input == nullptr && output == nullptr && period == 0 && !next_output && !next_period) return help();
        else throw std::runtime_error("_main(): Invalid usage");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    return _main(argc, argv);
}
