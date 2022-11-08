#pragma once

// https://www.heise.de/blog/Sleepy-Pico-ein-Raspberry-Pi-Pico-geht-mit-C-C-schlafen-6046517.html
// https://ghubcoder.github.io/posts/awaking-the-pico/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "hardware/structs/scb.h"
#include "hardware/clocks.h"
#include "extra/sleep.h"
#include "extra/rosc.h"

class Sleep
{
    public:
        static void sleep();
        static void recover();
        static void measure_freqs();
        static void setInterval(uint32_t v) { interval = v; }
        static void setDate(uint32_t _ymd, uint32_t _hms) { ymd = _ymd; hms = _hms; }

        static volatile bool awake;

    private:
        static uint scb_orig;
        static uint en0_orig;
        static uint en1_orig;

        static uint32_t ymd;                // yyyymmdd
        static uint32_t hms;                //   hhmmss
        static uint32_t interval;           // seconds

        static void rtc_sleep(); 
        static void alarm_callback();
};
