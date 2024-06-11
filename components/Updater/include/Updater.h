#ifndef UPDATER_H
#define UPDATER_H


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
//#include "CAN_file_transfer.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include <sys/unistd.h>
#include <string.h>
#include "CAN_file_transfer.h"
#include "TWAI.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int major;
    int minor;
    int patch;
} Version;

extern uint16_t versioneFW_esp1;
extern uint16_t versioneFW_esp3;
extern uint16_t versioneFW;

Version* getFirmwareVersions() ;
void getUpdateInfo() ;
bool checkupdate();
/*check if spiffs needs to be update
    Device :0 selft , 1 Display , 3 Driver
*/
bool updateSpiffs(uint8_t device);
//bool notify();
esp_err_t perform_ota_update(const char *file_path) ;

long getFileSize(const char* filePath) ;
bool performUpdate();

void TWAI_sendUpdateSize(uint32_t filesize);
void TWAI_sendUpdateInfo(Version* version);

#ifdef __cplusplus
}
#endif

#endif  //