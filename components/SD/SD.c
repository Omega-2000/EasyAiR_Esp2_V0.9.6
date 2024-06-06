#include "SD.h"

esp_err_t SD_init(Sd* sd, uint8_t miso, uint8_t mosi, uint8_t clk, uint8_t cs) {
    sd -> file_update = MOUNT_POINT"/update.txt";                   //  ... da fare dopo
    //sd -> file_temperature = MOUNT_POINT"/temperatures.txt";        //  fatto --> funziona bene ?
    sd -> file_temperature = MOUNT_POINT"/temperatures.csv";
    sd -> file_statistiche = MOUNT_POINT"/statistics.txt";
    sd -> file_log = MOUNT_POINT"/log.txt";                         //  ... da fare dopo
    sd -> file_valori = MOUNT_POINT"/values.txt";                   //  fatto --> funziona bene?

    sd -> MISO = miso;
    sd -> MOSI = mosi;
    sd -> CLK = clk;
    sd -> CS = cs;

    gpio_set_direction(sd -> CS, GPIO_MODE_OUTPUT);   //  spi
    gpio_set_level(sd -> CS, 1);

    sd -> busy = 0;

    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 29 * 1024
    };
    //  sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(SD_TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGI(SD_TAG, "Using SPI peripheral");

    //gpio_set_level(GPIO_NUM_14, 0);
    //esp_vfs_fat_sdcard_unmount(mount_point, sd -> card);
    //ESP_LOGI(SD_TAG, "Card unmounted");

    int ret_spi = SD_SPI_begin(sd);
    if (ret_spi != ESP_OK) {return ret_spi;}   ///////////////////////////////////////////////////////////////////

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = sd -> CS; //PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(SD_TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &sd -> card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SD_TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(SD_TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    ESP_LOGI(SD_TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, sd -> card);

    return ESP_OK;
}


esp_err_t SD_SPI_begin(Sd* sd) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = sd -> MOSI,      //PIN_NUM_MOSI,
        .miso_io_num = sd -> MISO,      //PIN_NUM_MISO,
        .sclk_io_num = sd -> CLK,       //PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    
    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(SD_TAG, "Failed to initialize bus.");
    }

    return ret;
}


bool SD_check(Sd* sd, const char *path, const char *object) {
    bool exists = false;

    sd -> dir = opendir(path);
    if (sd -> dir == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open directory");
        return ESP_FAIL;
    }

    struct dirent *entry;
    while ((entry = readdir(sd -> dir)) != NULL) {
        //  printf("entry->d_name: %s - ", entry->d_name);
        //  printf("object: %s\n", object);

        //printf("|%s|\n", entry->d_name);
        if(strcmp(entry->d_name, object) == 0) {
            exists = true;
            //ESP_LOGE(SD_TAG, "esiste");
        }
    }

    if (exists == true) { ESP_LOGE(SD_TAG, "%s already exists", object); } else { ESP_LOGI(SD_TAG, "%s doesn't exists, I'll create it", object); }

    closedir(sd -> dir);

    return exists;
}


esp_err_t SD_write(Sd* sd, uint8_t num_file, const char *string) {
    sd -> busy = 1;

    switch(num_file) {
        case 0:     //  update
            if (SD_check(sd, MOUNT_POINT, "update.txt") == false) {     //  se il file non esiste
                ESP_LOGI(SD_TAG, "writing %s...", sd -> file_update);
                sd -> f = fopen(sd -> file_update, "w");
            } else {
                ESP_LOGI(SD_TAG, "appending %s...", sd -> file_update);
                sd -> f = fopen(sd -> file_update, "a");
            }
            break;

        case 1:     //  temperature
            if (SD_check(sd, MOUNT_POINT, "temperatures.csv") == false) {
                ESP_LOGI(SD_TAG, "writing %s...", sd -> file_temperature);
                sd -> f = fopen(sd -> file_temperature, "w");
            } else {
                ESP_LOGI(SD_TAG, "appending %s...", sd -> file_temperature);
                sd -> f = fopen(sd -> file_temperature, "a");
            }
            break;

        case 2:     //  statistiche
            if (SD_check(sd, MOUNT_POINT, "statistics.txt") == false) {
                ESP_LOGI(SD_TAG, "writing %s...", sd -> file_statistiche);
                sd -> f = fopen(sd -> file_statistiche, "w");
            } else {
                ESP_LOGI(SD_TAG, "unlink\n");
                int res = unlink("/sdcard/statistics.txt");
                if (res == 0) {
                    ESP_LOGI(SD_TAG, "file 'statistics.txt' eliminato");
                    ESP_LOGI(SD_TAG, "writing %s...", sd -> file_statistiche);
                    ESP_LOGI(SD_TAG, "open\n");
                    sd -> f = fopen(sd -> file_statistiche, "w");
                    if (sd -> f == NULL) {
                        ESP_LOGE(SD_TAG, "ERRORE in open file statistiche");
                        break;
                    } else {
                        ESP_LOGI(SD_TAG, "open riuscito");
                    }
                } else {
                    ESP_LOGE(SD_TAG, "file 'statistics.txt' non eliminato");
                }
            }
            break;

        case 3:     //  log
            if (SD_check(sd, MOUNT_POINT, "log.txt") == false) {
                ESP_LOGI(SD_TAG, "writing %s...", sd -> file_log);
                sd -> f = fopen(sd -> file_log, "w");
            } else {
                ESP_LOGI(SD_TAG, "appending %s...", sd -> file_log);
                sd -> f = fopen(sd -> file_log, "a");
            }
            break;

        case 4:     //  valori
            if (SD_check(sd, MOUNT_POINT, "values.txt") == false) {
                sd -> f = fopen(sd -> file_valori, "w");
                
            } else {
                //  ESP_LOGI(SD_TAG, "unlink");
                int res = unlink("/sdcard/values.txt");
                if (res == 0) {
                    ESP_LOGI(SD_TAG, "file 'values.txt' eliminato");
                    //  ESP_LOGI(SD_TAG, "open");
                    sd -> f = fopen(sd -> file_valori, "w");
                    if (sd -> f == NULL) {
                        ESP_LOGE(SD_TAG, "ERRORE in open file valori");
                        break;
                    } else {
                        //  ESP_LOGI(SD_TAG, "open riuscito");
                    }
                } else {
                    ESP_LOGE(SD_TAG, "file 'values.txt' non eliminato");
                }
            }
            break;

        default:
            ESP_LOGE(SD_TAG, "File's number is wrong");
            sd -> busy = 0;
            return ESP_FAIL;
    }

    if (sd -> f == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open file for writing/appending");
        sd -> busy = 0;
        return ESP_FAIL;
    }

    if (sd -> f != NULL) {
        delay(10);
        //  ESP_LOGI(SD_TAG, "print");
        fprintf(sd -> f, "%s\n", string/*sd -> card->cid.name*/);
        fclose(sd -> f);
        ESP_LOGI(SD_TAG, "File written/appended");
    }

    sd -> busy = 0;
    return ESP_OK;
}


esp_err_t SD_read(Sd* sd, uint8_t num_file) {
    sd -> busy = 1;
    switch(num_file) {
        case 0:     //  update
            ESP_LOGI(SD_TAG, "Opening file %s", sd -> file_update);
            sd -> f = fopen(sd -> file_update, "r");
            break;
        case 1:     //  temperature
            ESP_LOGI(SD_TAG, "Opening file %s", sd -> file_temperature);
            sd -> f = fopen(sd -> file_temperature, "r");
            break;
        case 2:     //  statistiche
            ESP_LOGI(SD_TAG, "Opening file %s", sd -> file_statistiche);
            sd -> f = fopen(sd -> file_statistiche, "r");
            break;
        case 3:     //  log
            ESP_LOGI(SD_TAG, "Opening file %s", sd -> file_log);
            sd -> f = fopen(sd -> file_log, "r");
            break;
        case 4:     //  valori
            ESP_LOGI(SD_TAG, "Opening file %s", sd -> file_valori);
            sd -> f = fopen(sd -> file_valori, "r");
            break;
        default:
            ESP_LOGE(SD_TAG, "File's number is wrong");
            sd -> busy = 0;
            return ESP_FAIL;
    }

    if (sd -> f == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open file for reading");
        sd -> busy = 0;
        return ESP_FAIL;
    }

    while (fgets(SD_buffer_readed, sizeof(SD_buffer_readed), sd -> f) != NULL) {
        // Process or print the line as needed
        ESP_LOGI(SD_TAG, "%s", SD_buffer_readed);
    }

    fclose(sd -> f);
    ESP_LOGI(SD_TAG, "File readed\n");
    sd -> busy = 0;
    return ESP_OK;
}


esp_err_t SD_listDir(Sd* sd, const char *path) {
    ESP_LOGI(SD_TAG, "Opening directory %s", path);
    sd -> busy = 1;
    
    sd -> dir = opendir(path);
    if (sd -> dir == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open directory");
        sd -> busy = 0;
        return ESP_FAIL;
    }

    struct dirent *entry;
    while ((entry = readdir(sd -> dir)) != NULL) {
        ESP_LOGI(SD_TAG, "Entry: %s", entry->d_name);
    }

    closedir(sd -> dir);

    ESP_LOGI(SD_TAG, "Directory listed\n");
    sd -> busy = 0;
    return ESP_OK;
}


esp_err_t SD_operation(Sd* sd, bool operation, uint8_t n_file, char *s) {        //  operation == 0 -> READ  ,  operation == 1 -> WRITE
    esp_err_t ret = ESP_FAIL;
    if(!SD.busy) {
        if (operation) ret = SD_write(sd, n_file, s);
        else ret = SD_read(sd, n_file);
    } else {
        ESP_LOGE(TAG_sd, "SD occupata da un'altra operazione, riprovo...");
        bool scritto = false;
        while(!scritto) {
            if(!SD.busy) {
                if (operation) ret = SD_write(sd, n_file, s);
                else ret = SD_read(sd, n_file);
                scritto = true;
            }
            delay(10);
        }
    }
    return ret;
}
