// lightLogC.cpp V0.8 221112 qrt@qland.de

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

    sleep_ms(1000);
    
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
            sleep_ms(100);
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
