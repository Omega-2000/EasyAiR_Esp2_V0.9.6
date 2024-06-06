#include "CONFIG_FILE.h"

esp_err_t SPIFFS_init(ConfigData* config) {
    ESP_LOGI(SPIFF_TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SPIFF_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(SPIFF_TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(SPIFF_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

#ifdef CONFIG_EXAMPLE_SPIFFS_CHECK_ON_START
    ESP_LOGI(SPIFF_TAG, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFF_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return;
    } else {
        ESP_LOGI(SPIFF_TAG, "SPIFFS_check() successful");
    }
#endif

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFF_TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return ret;
    } else {
        ESP_LOGI(SPIFF_TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partiton size info.
    if (used > total) {
        ESP_LOGW(SPIFF_TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(SPIFF_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return ret;
        } else {
            ESP_LOGI(SPIFF_TAG, "SPIFFS_check() successful");
        }
    }

    config -> busy = 0;

    return ret;
}


esp_err_t SPIFFS_write_(ConfigData* config) {
    char char_millis_macchina[20];      snprintf(char_millis_macchina, sizeof(char_millis_macchina), "%ld", millis_macchina);
    char char_millis_pid[20];           snprintf(char_millis_pid, sizeof(char_millis_pid), "%ld", millis_pid); 
    char char_millis_motore[20];        snprintf(char_millis_motore, sizeof(char_millis_motore), "%ld", millis_motore); 

    char string[80];
    strcpy(string, char_millis_macchina); strcat(string, ";"); strcat(string, char_millis_pid); strcat(string, ";"); strcat(string, char_millis_motore); strcat(string, ";");

    config -> busy = 1;

    //esp_err_t res = esp_spiffs_format("storage");   //  ci mette 6 secondi o poco più
    uint8_t res = SPIFFS_remove_("/spiffs/millis.csv");

    if (res == ESP_FAIL) {
        ESP_LOGE(SPIFF_TAG, "ERRORE: file 'millis.csv' non eliminato");
        return res;
    } else if (res == ESP_OK) {
        ESP_LOGI(SPIFF_TAG, "file 'millis.csv' eliminato");
    } else if (res == 1) {
        ESP_LOGE(SPIFF_TAG, "file 'millis.csv' non esiste");
    }
    //delay(100);
    FILE* f = fopen("/spiffs/millis.csv", "w");
    if (f == NULL) {
        ESP_LOGE(SPIFF_TAG, "Failed to open file for writing");
        config -> busy = 0;
        return ESP_FAIL;
    }
    fprintf(f, "%s\n", string);
    fclose(f);
    ESP_LOGI(SPIFF_TAG, "File written");
    //t_spiff_aggiornato = millis();

    config -> busy = 0;

    return ESP_OK;
}


int8_t SPIFFS_read_(ConfigData* config) {
    //ESP_LOGI(SPIFF_TAG, "Reading file");

    config -> busy = 1;

    FILE* f = fopen("/spiffs/millis.csv", "r");
    if (f == NULL) {
        ESP_LOGE(SPIFF_TAG, "Failed to open file for reading");
        config -> busy = 0;
        return 1;
    }  else {
        ESP_LOGI(SPIFF_TAG,"File millis.csv opened !");
    }

    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    config -> busy = 0;

    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(SPIFF_TAG, "Read from file: '%s'", line);

    //  divisione della stringa e salvataggio millis salvati su variabili

    unsigned long millis[3];
    int cont = 0;
    char* token = strtok((char*)line, ";");
    while (token && (cont <= 2)) {

        char* endPtr;
        long longValue = strtol(token, &endPtr, 10); // 10 indicates base 10
        if (*endPtr == '\0') {
            millis[cont] = (int)longValue; // Optional cast to int
            //printf("intValue = %d\n", valori[cont]);
            // Now, intValue contains the integer value
        }
        //printf("Token: %s\n\n", token);
        token = strtok(NULL, ";");

        cont ++;        
    }

    millis_macchina += millis[0];        ESP_LOGI(SPIFF_TAG, "millis_macchina --> %ld", millis_macchina);
    millis_pid += millis[1];             ESP_LOGI(SPIFF_TAG, "millis_pid --> %ld", millis_pid);
    millis_motore += millis[2];          ESP_LOGI(SPIFF_TAG, "millis_motore --> %ld", millis_motore);

    return 0;
}

uint8_t SPIFFS_remove_(const char *filename) {
    ESP_LOGI(SPIFF_TAG, "Removing file: %s", filename);

    // Open the file for writing
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        ESP_LOGE(SPIFF_TAG, "Failed to open file for removal: %s", filename);
        return 1;
    }

    // Close the file
    fclose(file);

    // Remove the file
    if (unlink(filename) != 0) {
        ESP_LOGE(SPIFF_TAG, "Failed to remove file: %s", filename);
        return ESP_FAIL;
    }

    ESP_LOGI(SPIFF_TAG, "File removed successfully: %s", filename);
    return ESP_OK;
}
