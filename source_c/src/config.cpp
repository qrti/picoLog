#include "config.h"

struct Conf Config::cfg = {                     // default config
    .dateYMD = 20220101,
    .dateHMS = 0,
    .interval = 15,
    .append = false             
};

uint8_t Config::init()
{
    uint8_t err = FLASH_OK;

    // if(pico_mount(false) != LFS_ERR_OK){             // alread done in Sample::init()
    //     if(pico_mount(true) != LFS_ERR_OK)           // format flash
    //         err = FLASH_FORMAT_ERROR;
    //     else
    //         pico_unmount();
    // }

    if(getConfig() != FLASH_OK)
        err = setConfig();

    return err;
}

uint8_t Config::getConfig()
{
    uint8_t err = FLASH_OK;

    if(pico_mount(false) != LFS_ERR_OK){
        err = FLASH_MOUNT_ERROR;
    }
    else{
        int file = pico_open(CONFIG_FILE_NAME, LFS_O_RDONLY);

        if(file>=0 && pico_size(file)==sizeof(Conf)){
            pico_read(file, &cfg, sizeof(Conf));
            pico_close(file);
        }
        else{
            err = FLASH_FILE_ERROR;
        }

        pico_unmount();
    }

    return err;
}

uint8_t Config::setConfig()
{
    uint8_t err = FLASH_OK;

    if(pico_mount(false) != LFS_ERR_OK){
        err = FLASH_MOUNT_ERROR;
    }
    else{
        struct pico_fsstat_t stat;
        pico_fsstat(&stat);
        uint16_t blocksFree = stat.block_count - stat.blocks_used;

        if(blocksFree >= BLOCKS_MIN_FREE){
            int file = pico_open(CONFIG_FILE_NAME, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);

            if(file >= 0){
                pico_write(file, &cfg, sizeof(Conf));
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


    return err;
}
