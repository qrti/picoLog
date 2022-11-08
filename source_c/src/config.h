#pragma once

#include "extra/pico_hal.h"

#define CONFIG_FILE_NAME    "config.bin"

#define BLOCKS_MIN_FREE     2

#define FLASH_OK            0
#define FLASH_FULL_ERROR    1
#define FLASH_MOUNT_ERROR   2
#define FLASH_FILE_ERROR    3
#define FLASH_FORMAT_ERROR  4

typedef struct Conf{
    uint32_t dateYMD;                       // sample start yyyymmdd
    uint32_t dateHMS;                       //                hhmmss
    uint32_t interval;                      //        interval in seconds
    bool append;                            // append samples
}Conf;

class Config
{
    public:
        static uint8_t init();
        static void setDateYMD(uint32_t v) { cfg.dateYMD = v; }
        static void setDateHMS(uint32_t v) { cfg.dateHMS = v; }
        static void setInterval(uint32_t v) { cfg.interval = v; }
        static void setAppend(bool v) { cfg.append = v; }

        static uint32_t getDateYMD() { return cfg.dateYMD; }
        static uint32_t getDateHMS() { return cfg.dateHMS; }
        static uint32_t getInterval() { return cfg.interval; }
        static bool getAppend() { return cfg.append; }

        static void save() { setConfig(); }

    private:
        static struct Conf cfg;

        static uint8_t getConfig();
        static uint8_t setConfig();
};
