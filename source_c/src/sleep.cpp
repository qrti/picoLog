#include "sleep.h"

uint Sleep::scb_orig;
uint Sleep::en0_orig;
uint Sleep::en1_orig;
bool volatile Sleep::awake;
uint32_t Sleep::ymd;            
uint32_t Sleep::hms;          
uint32_t Sleep::interval;

void Sleep::sleep()
{
    scb_orig = scb_hw->scr;
    en0_orig = clocks_hw->sleep_en0;
    en1_orig = clocks_hw->sleep_en1;

    awake = false;
    sleep_run_from_xosc();
    rtc_sleep();
}

void Sleep::recover()
{
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = en0_orig;
    clocks_hw->sleep_en1 = en1_orig;

    clocks_init();
    // stdio_init_all();                            // recover stalls after about 5 cycles
}

void Sleep::rtc_sleep() 
{   
static bool once = false;
static datetime_t t_alarm;
static uint32_t alarm;

    if(!once){
        datetime_t t_ini = {                        // initial time
            .year  = (int16_t)(ymd / 10000),        // yyyymmdd -> yyyy
            .month = (int8_t)((ymd % 10000) / 100), //          -> mm
            .day   = (int8_t)(ymd % 100),           //          -> dd
            .dotw  = -1,                            // 0..6 sunday..saturday
            .hour  = (int8_t)(hms / 10000),         // hhmmss -> hh
            .min   = (int8_t)((hms % 10000) / 100), //        -> mm
            .sec   = (int8_t)(hms % 100)            //        -> ss
        };

        alarm = t_ini.hour * 3600 + t_ini.min * 60 + t_ini.sec;

        t_alarm = {                                 // alarm time
            .year  = -1,                            // -1 do not match
            .month = -1,
            .day   = -1,
            .dotw  = -1,            
            .hour  = 0, 
            .min   = 0,
            .sec   = 0
        };

        rtc_init();   
        rtc_set_datetime(&t_ini);
        once = true;
    }

    alarm = (alarm + interval) % 86400;             // max 1 day

    t_alarm.hour = interval>=3600 ? alarm/3600 : -1;
    t_alarm.min = interval>=60 ? alarm/60%60 : -1;
    t_alarm.sec = alarm % 60;

    sleep_goto_sleep_until(&t_alarm, &alarm_callback);
}

void Sleep::alarm_callback()
{
    awake = true;
}

void Sleep::measure_freqs() 
{
     uint f_pll_sys   = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
     uint f_pll_usb   = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
     uint f_rosc      = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
     uint f_clk_sys   = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
     uint f_clk_peri  = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
     uint f_clk_usb   = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
     uint f_clk_adc   = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
     uint f_clk_rtc   = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

     printf("pll_sys  = %dkHz\n", f_pll_sys);
     printf("pll_usb  = %dkHz\n", f_pll_usb);
     printf("rosc     = %dkHz\n", f_rosc);
     printf("clk_sys  = %dkHz\n", f_clk_sys);
     printf("clk_peri = %dkHz\n", f_clk_peri);
     printf("clk_usb  = %dkHz\n", f_clk_usb);
     printf("clk_adc  = %dkHz\n", f_clk_adc);
     printf("clk_rtc  = %dkHz\n", f_clk_rtc);
   
     uart_default_tx_wait_blocking(); // wait blocking for UART output 
}
