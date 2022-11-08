// lightLogC.cpp V0.6 221108 qrt@qland.de

// the power consumption in sleep mode with running RTC at 3 V is about 1.2 mA
// dependent on the sample interval two alkaline batteries (~2.800 mAh) may endure some month

// commands
// sample       start sampling with current settings
//              to stop sampling see point save stop
//
// dump         dump saved samples
//
// remove       remove sampled data
//
// format       format pico flash
//
// checkADC     show current ADC value
//
// set_ymd      sample start date, yyyymmdd
//
// set_hms                         hhmmss      
//
// set_interval sample interval                         1..86400 seconds (1 day)
//
// set_append   append sampled data to existing ones    0 new data file, 1 append data to existing file

// buttons
// reset        between pico run pin (30) and ground
//              pressed to stop sampling, preferably between samples 
//
// start        between start pin (-> START_PIN) and ground
//              (a capacitor and 47 ohm resistor in series parallel to the button is advised)
//              if pressed while power up       -> start sampling
//              if not pressed while power up   -> USB serial for script communication 
//              also see point save stop

// LED blink codes
// menu         constantly on
//
// sampling     1 short blink on every sample and same as on powerup for errors
//
// powerup      2 short blinks and a pause endless on FLASH_FULL_ERROR
//              3                                     FLASH_MOUNT_ERROR
//              4                                     FLASH_FILE_ERROR
//              5                                     FLASH_FORMAT_ERROR  
//
// block        short blinks if a sample is scheduled while start button is pressed
//              see point save stop

// save stop
// stop sampling by pressing reset
// avoid unvalid flash writes by pressing reset between samples
// the save procedure is to hold the start button then pressing reset, then releasing start and then reset

// remarks
// - after waking up from sleep USB serial printf does not work anymore
//   if you need printf outputs
//   #define QUART          1
//   for pin configuration to a USB serial (FTDI) device see quart.h 
// - for adc pin configuartion see sample.h
// - to test without sleep
//   #define USE_SLEEP      0

// edited and compiled with 
// https://code.visualstudio.com/
// https://github.com/Wiz-IO/wizio-pico
// https://platformio.org/

// based and inspired on works of
// https://github.com/littlefs-project/littlefs
// https://github.com/lurk101/pico-littlefs -> https://github.com/litten2up/pico-littlefs
// https://www.heise.de/blog/Sleepy-Pico-ein-Raspberry-Pi-Pico-geht-mit-C-C-schlafen-6046517.html
// https://ghubcoder.github.io/posts/awaking-the-pico/

#include <stdio.h>
#include "pico/stdlib.h"
#include "sleep.h"
#include "sample.h"
#include "config.h"
#include "quart.h"

#define START_PIN       2           // sample start pin
#define QUART           0           // 0 off, 1 active
#define USE_SLEEP       1           // 0 common delay (~20 mA), 1 sleep (~1.2 mA)

void sample();
void dump();
void remove();
void format();
void checkADC();
void setDateYMD(uint32_t yyymmdd);
void setDateHMS(uint32_t hhmmss);
void setInterval(uint32_t interval);
void setAppend(bool append);

void signal(uint8_t wink, bool forever);

int main(void)
{  
    stdio_init_all();    

    #if QUART == 1
        QUart quart;
        quart.init();
    #endif

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    gpio_init(START_PIN);
    gpio_set_dir(START_PIN, GPIO_IN);
    gpio_pull_up(START_PIN);

    sleep_ms(3000);
    
    uint8_t err;

    if((err = Sample::init()) != FLASH_OK)
        signal(err + 1, true);    

    if((err = Config::init()) != FLASH_OK)
        signal(err + 1, true);    

    if(gpio_get(START_PIN) == 0)            // start sampling if button pressed
        sample();                   

    printf("ready\n");

    char cmd[32];
    uint32_t par;

    while(true){
        scanf("%s %lu", cmd, &par);

        if(strcmp(cmd, "sample") == 0){
            sample();
        }
        else if(strcmp(cmd, "dump") == 0){
            dump();
        }
        else if(strcmp(cmd, "remove") == 0){
            remove();
        }
        else if(strcmp(cmd, "format") == 0){
            format();
        }
        else if(strcmp(cmd, "checkadc") == 0){
            checkADC();
        }
        else if(strcmp(cmd, "set_date") == 0){
            setDateYMD(par);
        }
        else if(strcmp(cmd, "set_time") == 0){
            setDateHMS(par);
        }
        else if(strcmp(cmd, "set_interval") == 0){
            setInterval(par);
        }
        else if(strcmp(cmd, "set_append") == 0){
            setAppend((bool)par);
        }
        else if(strcmp(cmd, "test") == 0){
            printf("cmd=%s par=%lu\n", cmd, par);
        }
        else{
            printf("???\n");
        }
    }
}

void sample()
{
    uint8_t err;

    printf("OK\n");
    gpio_put(PICO_DEFAULT_LED_PIN, 0);          // LED off
    sleep_ms(1000);

    uint32_t v = Config::getInterval();         // interval  1..30 s -> 42..60 s save interval
    Sample::setBufSize(v<31 ? 60/v : 1);        //          31..     -> 31..    
    Sleep::setInterval(v);                          
    Sleep::setDate(Config::getDateYMD(), Config::getDateHMS());

    if(!Config::getAppend())                    // if not append
        Sample::remove();                       // remove data file

    while(true){
        while(gpio_get(START_PIN) == 0){        // block sampling while button is pressed
            signal(1, false);                   // blink while blocking
            sleep_ms(250);
        }

        err = Sample::sample();                 // sample
        signal(err + 1, false);                 // blink code

        #if USE_SLEEP == 1                      // real sleep
            Sleep::sleep();                     // sleep

            while(!Sleep::awake)                // wake up
                tight_loop_contents();

            Sleep::recover();                   // recover

            #if QUART == 1                      // reinit uart
                uart_init(uart0, 115200);           
            #endif
        #elif USE_SLEEP == 0                    // common delay
            sleep_ms(v * 1000);
        #endif    
    }
}

void dump()
{
uint8_t err;    
int32_t size;

    if((err = Sample::dump(&size)) != FLASH_OK){
        if(err == FLASH_MOUNT_ERROR)
            printf("error: mount failed\n");
        else if(err == FLASH_FILE_ERROR) 
            printf("error: invalid data file\n");
    }
    else{
        printf("%08lu %06lu %06lu %ld\n", Config::getDateYMD(), Config::getDateHMS(), Config::getInterval(), size/2);
    }
}

void remove()
{
uint8_t err;    

    if((err = Sample::remove()) != FLASH_OK){
        if(err == FLASH_MOUNT_ERROR)
            printf("error: mount failed\n");
        else if(err == FLASH_FILE_ERROR) 
            printf("error: no data file\n");
    }
    else{
        printf("OK\n");        
    }
}

void format()
{
    Sample::format();            
    printf("OK\n");
}

void setDateYMD(uint32_t yyyymmdd)
{
    Config::setDateYMD(yyyymmdd);    
    Config::save();
    printf("OK\n");
}

void setDateHMS(uint32_t hhmmss)
{
    Config::setDateHMS(hhmmss);    
    Config::save();
    printf("OK\n");
}

void setInterval(uint32_t interval)
{
    Config::setInterval(interval);    
    Config::save();
    printf("OK\n");
}

void setAppend(bool append)
{
    Config::setAppend(append);
    Config::save();
    printf("OK\n");    
}

void checkADC()
{
    printf("0x%04x\n", adc_read());
}

void signal(uint8_t blink, bool forever)
{
    do{
        for(uint8_t i=0; i<blink; i++){
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(10);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);

            if(i != blink-1)
                sleep_ms(250);
        }

        if(forever)
            sleep_ms(1000);
      
    }while(forever);
}

// uart_default_tx_wait_blocking();
