#include "sample.h"

uint16_t* Sample::sBuf;
uint8_t Sample::sBufSize;
uint8_t Sample::sbi;

uint8_t Sample::init()
{
    uint8_t err = FLASH_OK;
    sBufSize = 1;

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    if(pico_mount(false) != LFS_ERR_OK){
        if(pico_mount(true) != LFS_ERR_OK)                  // format flash
            err = FLASH_FORMAT_ERROR;
        else
            pico_unmount();
    }

    return err;
}

// size of buffer to collect samples before saving to flash
//
void Sample::setBufSize(uint8_t size)                          
{
    if(sBuf) free(sBuf);
    sBuf = (uint16_t*)malloc(size * SAMPLE_BYTES);
    sBufSize = size;
    sbi = 0;
}

uint8_t Sample::sample()
{    
    uint8_t err = FLASH_OK;

    sBuf[sbi++] = adc_read();

    if(sbi >= sBufSize){
        if(pico_mount(false) != LFS_ERR_OK){
            err = FLASH_MOUNT_ERROR;
        }
        else{
            struct pico_fsstat_t stat;
            pico_fsstat(&stat);
            uint16_t blocksFree = stat.block_count - stat.blocks_used;

            if(blocksFree >= BLOCKS_MIN_FREE){
                int file = pico_open(SAMPLE_FILE_NAME, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);

                if(file >= 0){
                    pico_write(file, sBuf, sBufSize * SAMPLE_BYTES);
                    pico_close(file);
                }
                else{
                    err = FLASH_FILE_ERROR;
                }
            }
            else{
                err = FLASH_FULL_ERROR;
            }

            pico_unmount();
        }

        sbi = 0;
    }

    return err;
}

uint8_t Sample::dump(int32_t* size)
{
    uint8_t err = FLASH_OK;

    if(pico_mount(false) != LFS_ERR_OK){
        err = FLASH_MOUNT_ERROR;
    }
    else{
        int file = pico_open(SAMPLE_FILE_NAME, LFS_O_RDONLY);

        if(file >= 0){
            uint16_t buf[DUBLWI];
            int32_t s = pico_size(file);
            *size = s;

            while(s){
                uint16_t a = s>=DUBLWI*2 ? DUBLWI*2 : s;
                pico_read(file, buf, a);

                for(uint16_t i=0; i<a/2; i++)
                    printf("0x%04x ", buf[i]);

                printf("\n");
                s -= a;
            }
        
            pico_close(file);
        }
        else{
            err = FLASH_FILE_ERROR;
        }

        pico_unmount();
    }

    return err;
}

uint8_t Sample::remove()
{
    uint8_t err = FLASH_OK;

    if(pico_mount(false) != LFS_ERR_OK){
        err = FLASH_MOUNT_ERROR;
    }
    else{
        if(pico_remove(SAMPLE_FILE_NAME) < 0)
            err = FLASH_FILE_ERROR;

        pico_unmount();
    }

    return err;
}

uint8_t Sample::format()
{
    uint8_t err = FLASH_OK;

    if(pico_mount(true) != LFS_ERR_OK)                  // format flash
        err = FLASH_FORMAT_ERROR;
    else
        pico_unmount();

    return err;
}
