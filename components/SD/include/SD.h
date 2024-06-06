#ifndef __SD_H__
#define __SD_H__


#define SD_TAG     "_SD_"
#define delay(ms)   vTaskDelay(ms/portTICK_PERIOD_MS)
#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO    13
#define PIN_NUM_MOSI    11
#define PIN_NUM_CLK     12
#define PIN_NUM_CS      10


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <dirent.h>
/*#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"*/

static const char *TAG_sd = "_Sd_";

typedef struct {
    uint8_t MISO, MOSI, CLK, CS;
    sdmmc_card_t *card;

    FILE *f;
    const char *file_update;        // 0
    const char *file_temperature;   // 1
    const char *file_statistiche;   // 2
    const char *file_log;           // 3
    const char *file_valori;        // 4

    DIR *dir;

    bool busy;
} Sd;

extern Sd SD;
extern bool SD_works;
extern char SD_buffer_readed[256];
extern unsigned long t_statistiche_in_SD;
extern bool FAILED_write_statistiche_in_SD;

esp_err_t SD_init(Sd* sd, uint8_t miso, uint8_t mosi, uint8_t clk, uint8_t cs);
esp_err_t SD_SPI_begin(Sd* sd);
esp_err_t SD_write(Sd* sd, uint8_t num_file, const char *string);
esp_err_t SD_read(Sd* sd, uint8_t num_file);
esp_err_t SD_empty(Sd* sd, uint8_t num_file);
esp_err_t SD_rename(Sd* sd, uint8_t num_file, const char *string);
void SD_mkdir(Sd* sd, const char *path, const char *string);
esp_err_t SD_listDir(Sd* sd, const char *path);
bool SD_check(Sd* sd, const char *path, const char *object);
void SD_read_statistics();      //  ORE DI UTILIZZO
esp_err_t SD_operation(Sd* sd, bool operation, uint8_t n_file, char *s);

#ifdef __cplusplus
}
#endif

#endif  // __SD_H__
