#include "Updater.h"
//#include "CAN_file_transfer.h"

#define TAG "UPDATER: "


Version* getFirmwareVersions() {
    const char *filename = "/sdcard/updates/versions.txt";
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open the file %s", filename);
        return NULL;
    }

    char version[12]; // Assuming the version format is always 100.100.100
    if (fgets(version, sizeof(version), file) == NULL) {
        printf("Error reading file");
        fclose(file);
        return NULL;
    }

    fclose(file);

    // Extracting each number
    int displayVersion, driverVersion, espServiceVersion;
    if (sscanf(version, "%d.%d.%d", &displayVersion, &driverVersion, &espServiceVersion) != 3) {
        printf("Error parsing version");
        return NULL;
    }

    // Interpret the version numbers
    Version* versions = (Version*)malloc(3 * sizeof(Version));

    versions[0].major = displayVersion; //displayVersion / 100;
    versions[0].minor = 0;//(displayVersion / 10) % 10;
    versions[0].patch = 0; //displayVersion % 10;

    versions[1].major = driverVersion;//driverVersion / 100;
    versions[1].minor = 0;//(driverVersion / 10) % 10;
    versions[1].patch = 0;//driverVersion % 10;

    versions[2].major = espServiceVersion;//espServiceVersion / 100;
    versions[2].minor = 0;// (espServiceVersion / 10) % 10;
    versions[2].patch = 0;//espServiceVersion % 10;

    return versions;
}

bool checkupdate() {
    Version* versions = getFirmwareVersions();
        
    if (versions != NULL) {
        printf("Display Firmware: %d.%d.%d\n", versions[0].major, versions[0].minor, versions[0].patch);
        printf("Driver Firmware: %d.%d.%d\n", versions[1].major, versions[1].minor, versions[1].patch);
        printf("ESP Service Firmware: %d.%d.%d\n", versions[2].major, versions[2].minor, versions[2].patch);

        // Don't forget to free the allocated memory
        free(versions);
    }
  //TODo implement here Check if current version == version on the file 
    if(versioneFW_esp1 < versions[0].major || versioneFW_esp3 < versions[1].major || versioneFW <  versions[2].major){

        return true;
    }
    return false;    
}

bool updateSpiffs(uint8_t device) {
    const char *filename = "/sdcard/updates/spiffs.txt";
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open the file %s", filename);
        return false;
    }

    char version[8]; // Assuming the version format is always 1.0.1
    if (fgets(version, sizeof(version), file) == NULL) {
        printf("Error reading file");
        fclose(file);
        return false;
    }

    fclose(file);

    // Extracting the SPIFFS version
    int major, minor, patch;
    if (sscanf(version, "%d.%d.%d", &major, &minor, &patch) != 3) {
        printf("Error parsing SPIFFS version");
        return false;
    }

    // Determine if SPIFFS needs to be updated for the specified device
    if (device == 0 && major == 1) {
        return true;
    } else if (device == 1 && minor == 1) {
        return true;
    } else if (device == 2 && patch == 1) {
        return true;
    }

    return false;
}

long getFileSize(const char* filePath) {
    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        printf("Could not open the file");
        return -1; // Return -1 to indicate an error
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

void getUpdateInfo() {
         ESP_LOGI(TAG, "update Display size %ld",getFileSize("/sdcard/updates/Display/firmware.bin"));
         ESP_LOGI(TAG, "update Driver size %ld",getFileSize("/sdcard/updates/Driver/firmware.bin"));
        if(updateSpiffs(1)){
            ESP_LOGI(TAG, "update Spiff self size %ld",getFileSize("/sdcard/updates/versions.txt"));
        }
         
}

bool performUpdate() {
    createNewCanFileTransfer(SENDER_MODE);
    Version* firmwareVersion = getFirmwareVersions();

    if (firmwareVersion[0].major > versioneFW) {
        // Do self update
        CAN_sendFileTransfertInfo(0x00,0); // Send Starting updated status
        ESP_LOGI(TAG," Permform self update ...");
        perform_ota_update("/sdcard/updates/Self/firmware.bin");
        CAN_sendFileTransfertInfo(0x00,1); // Send finished updated status

    }
    
    if (firmwareVersion[1].major > versioneFW_esp1) {
        // Do Display update
        ESP_LOGI(TAG," Permform Display update ...");

        if(!isExistingTransferProcess()) {
            CAN_sendFileTransfertInfo(0x01,0); // Send Starting updated status
            if (divideFileIntoChunks("/sdcard/updates/Display/firmware.bin") == ESP_OK) {
                ESP_LOGI(TAG,"File firmware display transfered");
                CAN_sendFileTransfertInfo(0x01,1); // Send Starting updated status
                if (updateSpiffs(1)) {
                    ESP_LOGI(TAG," Permform Display SPIFFS update ...");
                    if(divideFileIntoChunks("/sdcard/updates/Display/Data/data.json") == ESP_OK) {
                        ESP_LOGI(TAG,"Display SPIFFS update SUCCESSFUL");
                    } else {
                        ESP_LOGE(TAG,"Error Display's SPIFFS update ");
                    }
                }
            } else {
                ESP_LOGE(TAG,"Error Display update");
                return false;
            }
        }
    }

    if (firmwareVersion[2].major > versioneFW_esp3) {
        // Do Driver update
        ESP_LOGI(TAG," Permform Driver update ...");

        if(!isExistingTransferProcess()){
            CAN_sendFileTransfertInfo(0x02,0); // Send Starting updated status

            if(divideFileIntoChunks("/sdcard/updates/Driver/firmware.bin")==ESP_OK){
            CAN_sendFileTransfertInfo(0x02,1); // Send Starting updated status

                ESP_LOGI(TAG,"File firmware driver transfered");
                        if(updateSpiffs(1)){
                                ESP_LOGI(TAG," Permform Driver SPIFFS update ...");
                                if(divideFileIntoChunks("/sdcard/updates/Driver/Data/data.json")==ESP_OK){
                                        ESP_LOGI(TAG,"Driver SPIFFS update SUCCESSFUL");
                                }else{
                                    ESP_LOGE(TAG,"Error Driver SPIFFS update ");
                            }

                        }
            }else{
                ESP_LOGE(TAG,"Error Driver update");
                return false;
            }
        }

    }

    CAN_sendFileTransfertInfo(0x03,1); // Notify Update
    return true;
}

esp_err_t perform_ota_update(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    ESP_LOGI(TAG, "Update path %s", file_path);
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open the firmware file");
        return ESP_FAIL;
    }

    // Check the running partition
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    if (strcmp(running_partition->label, "factory") != 0 && strcmp(running_partition->label, "ota_0") != 0 && strcmp(running_partition->label, "ota_1") != 0) {
        ESP_LOGE(TAG, "Running partition label mismatch. Expected 'factory', 'ota_0', or 'ota_1'.");
        fclose(file);
        return ESP_ERR_INVALID_STATE;
    }

    // Find the update partition
    const esp_partition_t *update_partition;
    if (strcmp(running_partition->label, "factory") == 0) {
        update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    } else {
        update_partition = esp_ota_get_next_update_partition(NULL);
    }
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "Failed to get the update partition");
        fclose(file);
        return ESP_FAIL;
    }

    esp_ota_handle_t ota_handle;
    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error starting OTA update. Error 0x%x", err);
        fclose(file);
        return err;
    }

    char data[1024];
    size_t read_bytes;
    ESP_LOGI(TAG, "Start writing OTA...");
    while ((read_bytes = fread(data, 1, sizeof(data), file)) > 0) {
        err = esp_ota_write(ota_handle, data, read_bytes);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error writing OTA data. Error 0x%x", err);
            fclose(file);
            return err;
        }
    }

    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error ending OTA update. Error 0x%x", err);
        fclose(file);
        return err;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting boot partition. Error 0x%x", err);
        fclose(file);
        return err;
    }

    fclose(file);

    ESP_LOGI(TAG, "Update file should be written in OTA, performing partition validation");

    // Validate the new firmware
        running_partition = esp_ota_get_running_partition();
    if (running_partition != NULL && update_partition != NULL) {
        // Check label assignment for running and updated partitions
        ESP_LOGI(TAG, "Running Partition Label: %s", running_partition->label);
        ESP_LOGI(TAG, "Updated Partition Label: %s", update_partition->label);

        if (strcmp(running_partition->label, "factory") == 0 && strcmp(update_partition->label, "ota_0") == 0) {
            ESP_LOGI(TAG, "Initial OTA update. No need for validation check.");
        } else if((strcmp(running_partition->label, "ota_0") == 0 && strcmp(update_partition->label, "ota_1") == 0) ||
                         (strcmp(running_partition->label, "ota_1") == 0 && strcmp(update_partition->label, "ota_0") == 0)) {
             ESP_LOGI(TAG, "OTA partition validation failed. Labels do not match.");
            //return ESP_ERR_OTA_VALIDATE_FAILED;
        }
        else{
            ESP_LOGE(TAG, "OTA partition validation failed. Check running partition and updated partition");
            return ESP_ERR_OTA_VALIDATE_FAILED;
        }
    }

    ESP_LOGI(TAG, "Firmware update successful. Restarting...");
    //esp_restart();

    return ESP_OK;
}

void TWAI_sendUpdateSize(uint32_t filesize) {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 8;
    uint8_t buffer[len];    // ={0};    ?????????
    buffer[0]= 0x07;
    buffer[1]= 0x04;
    buffer[2]= 0x00;
    buffer[3]= 0x01;
    // Populate buffer[4] to buffer[7] with the value of filesize
    buffer[4] = (filesize >> 24) & 0xFF; // Most significant byte
    buffer[5] = (filesize >> 16) & 0xFF;
    buffer[6] = (filesize >> 8) & 0xFF;
    buffer[7] = filesize & 0xFF; // Least significant byte

    //CAN_FT_sendMessage(0x111,buffer);
    TWAI_send(id, FLAG, len, buffer);
}

void TWAI_sendUpdateInfo(Version* version) {
    uint8_t *id = COMMAND_ID | ESP2_M_ID | ESP1_D_ID;
    uint8_t len = 5;
    uint8_t buffer[len];
    buffer[0]= 0x07;
    buffer[1]= 0x02;
    buffer[2]= version[1].major;
    buffer[3]= version[0].major;
    buffer[4]=version[2].major ;

    //CAN_FT_sendMessage(0x111,buffer);
    TWAI_send(id, FLAG, len, buffer);
}
