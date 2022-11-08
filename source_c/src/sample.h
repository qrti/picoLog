#pragma once

#include "hardware/adc.h"
#include "extra/pico_hal.h"

#define ADC_PIN             26      // ADC0

#define SAMPLE_FILE_NAME    "data.bin"

#define BLOCKS_MIN_FREE     2
#define DUBLWI              16      // dump block width in 2 byte words
#define SAMPLE_BYTES        2       // 2 byte word

#define FLASH_OK            0
#define FLASH_FULL_ERROR    1
#define FLASH_MOUNT_ERROR   2
#define FLASH_FILE_ERROR    3
#define FLASH_FORMAT_ERROR  4

class Sample
{
    public:
        static uint8_t init();
        static uint8_t sample();
        static uint8_t dump(int32_t* size);
        static uint8_t remove();
        static uint8_t format();
        static void setBufSize(uint8_t size);

    private:
        static uint16_t* sBuf;          // sample buffer
        static uint8_t sBufSize;        //               size  in 2 byte words
        static uint8_t sbi;             //               index
};
