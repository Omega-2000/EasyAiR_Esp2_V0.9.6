#include "CAN_file_transfer.h"
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_crc.h"
#include "esp_spiffs.h"
//#include "esp_timer.h"
#include "C:\Users\alessandro\esp\esp-idf\components\esp_timer\include\esp_timer.h"
#include "dirent.h"
#include "string.h"
#define TAG "FILE_TRANSFER_CAN: "

#define CANBUS_SOURCE_MASK 0x0F0  // Correct mask for bits 4-7

void listFilesInSPIFFS() {
    struct dirent *de;
    DIR *dr = opendir("/spiffs");

    if (dr == NULL) {
        printf("Could not open current directory" );
        return;
    }

    while ((de = readdir(dr)) != NULL) {
        printf("%s\n", de->d_name);
    }

    closedir(dr);
}
 
//Twai twai;   
int chunkCount = 0;
int MaxChunk=0;
FILE *outputFile ;
long fileSize;
uint32_t expectedFileSize;

const char *outputDirectory = "/spiffs/"; // Adjust the directory as needed
char outputFilename[100] = ""; // Adjust the array size as needed
int indeX = 0;

//uint8_t canbusSourceID = 0;
uint8_t mode =0 ;// 0 sender , 1 receiver 
bool existingTransfertProcess = false ;
unsigned int timeout=100000;
TaskHandle_t receiverTaskHandle= NULL;

struct CANFileTransfer *myCanFileTransferHandler = NULL;

static UpdateStateChangedHandler updateStateChangedHandler = NULL;

void registerUpdateStateChangedHandler(UpdateStateChangedHandler handler) {
    updateStateChangedHandler = handler;
}

void processUpdateState(int device, int state) {

    // Notify the registered callback about the update state change
    if (updateStateChangedHandler != NULL) {
        updateStateChangedHandler(device, state);
    }
}

struct CANFileTransfer* createNewCanFileTransfer(){

    struct CANFileTransfer* CanFileTransfer = malloc(sizeof(struct CANFileTransfer));
    if (CanFileTransfer == NULL) {
        // Handle memory allocation failure
        return NULL;
    }
    CanFileTransfer->chunkCount=0;
    CanFileTransfer->MaxChunk=0;
    //CanFileTransfer->outputFile;
    CanFileTransfer->fileSize=0;
    CanFileTransfer->expectedFileSize=0;
    CanFileTransfer->outputFilename=NULL;
    CanFileTransfer->canbusSourceID=0;
    CanFileTransfer->mode=RECEIVER_MODE;
    CanFileTransfer->existingTransferProcess =false;
    CanFileTransfer->timeout=1000000;
    CanFileTransfer->receiverTaskHandle=NULL;
   // Assign the created CanFileTransfer to the global variable
    myCanFileTransferHandler = CanFileTransfer;
    return CanFileTransfer;
}

void CAN_FT_messageListner(void *pvParameters) {
    CAN_FT_initCanbus();
    while (1) {
        if(TWAI_get() == ESP_OK) {
            //uint8_t ret = command_receiver();
            //printf("ret = %d\n", ret);
            if (idx == UNIVERSAL_ID) {}
            
            if (myCanFileTransferHandler->mode==SENDER_MODE) {
                analyzeMessagesFileSender(message);
            } else if (myCanFileTransferHandler->mode==RECEIVER_MODE) {
                analyzeMessagesFileReceiver(message);
            }
        }
        // Task code here
        //ESP_LOGI("CAN_BUS_TAsk", "Running...");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
 
int CAN_FT_initCanbus(){
  if ( TWAI_init() == ESP_OK ) {
        printf("twai initialization --> OK\n");
        return 0;

    } else {
        printf("twai initialization --> FAIL\n");
        return 1;

    }

}

void CAN_FT_sendMessage(uint32_t id, uint8_t data[]){
    
    TWAI_send( id, FLAG, 8,  data);
    //printf("send\n");
}

// Function to get the free space in SPIFFS
void checkSpiffsFreeSpace() {
    size_t total, used;
    esp_spiffs_info(NULL, &total, &used);
    printf("Total SPIFFS space: %d bytes\n", total);
    printf("Used SPIFFS space: %d bytes\n", used);
    printf("Free SPIFFS space: %d bytes\n", total - used);
}

void analyzeMessagesFileReceiver(twai_message_t message){

if(message.identifier!=0) {

    printf("ID: %#lx ", (unsigned long)message.identifier);

    // uint32_t identifier = message.identifier;
    uint8_t priority = (message.identifier >> 8) & 0x7; // Extract the most significant byte (bits 24-26)
    uint8_t source = (message.identifier >> 4) & 0xF;   // Extract the second byte (bits 16-19)
    uint8_t dest = message.identifier  & 0xF;          // Extract the third byte (bits 0-3)

    

    //printf("ID: %#lx [ ", (unsigned long)message.identifier);
    uint8_t buf[LEN];   
    if (!(message.flags & TWAI_MSG_FLAG_RTR)) {
        for (int i = 0; i < message.data_length_code; i++) {
            printf("%02x ", message.data[i]);
            buf[i] = message.data[i];
        }
        //  printf("  canbus.cpp]\n");
        ESP_LOGI(TAG,"priority:  %d",priority);
        ESP_LOGI(TAG,"source:  %d",source);
        ESP_LOGI(TAG,"dest:  %d",dest);
                
    }
    switch (message.identifier) {
    case INIT_TRANSMISSION:
                
                // Reconstruct the 32-bit integer
                myCanFileTransferHandler->MaxChunk |= ((int32_t)buf[0] << 24);
                myCanFileTransferHandler->MaxChunk |= ((int32_t)buf[1] << 16);
                myCanFileTransferHandler->MaxChunk |= ((int32_t)buf[2] << 8);
                myCanFileTransferHandler->MaxChunk |= buf[3];
 
                myCanFileTransferHandler->expectedFileSize |= ((int32_t)buf[4] << 24);
                myCanFileTransferHandler->expectedFileSize |= ((int32_t)buf[5] << 16);
                myCanFileTransferHandler->expectedFileSize |= ((int32_t)buf[6] << 8);
                myCanFileTransferHandler->expectedFileSize |= buf[7];
                 ESP_LOGI(TAG,"Init Chunk info %d",myCanFileTransferHandler->MaxChunk);

                myCanFileTransferHandler->outputFilename= "/spiffs/firmware.bin";
                myCanFileTransferHandler->outputFile = fopen(myCanFileTransferHandler->outputFilename, "wb");
                ESP_LOGI(TAG,"File transfer init with total chunk number %d",myCanFileTransferHandler->MaxChunk);
                myCanFileTransferHandler->existingTransferProcess=true;
        break;
    case END_TRANSMISSION:
            fclose(myCanFileTransferHandler->outputFile);
            myCanFileTransferHandler->outputFile = fopen(myCanFileTransferHandler->outputFilename, "rb");

           if (myCanFileTransferHandler->outputFile != NULL) {
               // Get the file size
            fseek(myCanFileTransferHandler->outputFile, 0, SEEK_END);
            myCanFileTransferHandler->fileSize = ftell(myCanFileTransferHandler->outputFile);
            ESP_LOGI(TAG,"Received file size %ld",  myCanFileTransferHandler->fileSize);
           fclose(myCanFileTransferHandler->outputFile);
      }
      else{
        perror("Error");
        listFilesInSPIFFS();
        ESP_LOGI(TAG,"file null %s",myCanFileTransferHandler->outputFilename);
      }




             if(myCanFileTransferHandler->chunkCount!=myCanFileTransferHandler->MaxChunk){
            ESP_LOGE(TAG,"Missing chunk, received chunk = %d or Excpected chunk nbr = %d \n missing %d chunks",chunkCount,myCanFileTransferHandler->MaxChunk,myCanFileTransferHandler->chunkCount-myCanFileTransferHandler->MaxChunk);

          }

            if(buf[0]== 0){
                ESP_LOGI(TAG,"File transmission failed or aborted") ;
                myCanFileTransferHandler->existingTransferProcess=false;
                break;
            } 
            else if(buf[0]== 1){
                ESP_LOGI(TAG,"Transfer file Successful!") ;
                myCanFileTransferHandler->existingTransferProcess=false;
                break;
                //TODO : Do action after file transferered
               // perform_ota_update(myCanFileTransferHandler->outputFilename);
            }
          
        
         
         
        break;
    case CHUNK_TRANSFERT:
           ESP_LOGI(TAG,"Update chunck %d",myCanFileTransferHandler->chunkCount);
         if (myCanFileTransferHandler->chunkCount <= myCanFileTransferHandler->MaxChunk -1 ) {
  
            // Extract the CRC8 value from the chunk
            uint8_t received_crc = message.data[DATA_SIZE];

             // Calculate the CRC of the received chunk data
             uint8_t calculated_crc = esp_crc8_le(0, message.data, DATA_SIZE);
           
            // Compare the two CRC values
            if (received_crc == calculated_crc) {
                 CAN_sendAction(1);
                fwrite(message.data, 1, DATA_SIZE, &myCanFileTransferHandler->outputFile);
               
               
                /*
                for (size_t i = 0; i < DATA_SIZE; i++) {
                    printf("%02x ", message.data[i]);
                    chunks[myCanFileTransferHandler->chunkCount][i] = message.data[i];
                }*/
                myCanFileTransferHandler->chunkCount++;
            } else {
                 CAN_sendAction(3);
                ESP_LOGE(TAG, "CRC mismatch, discarding chunk. Received CRC = %d, Calculated CRC = %d and message.data_length_code = %d", received_crc, calculated_crc , message.data_length_code);
            }
        }else{
            CAN_sendAction(0);
            ESP_LOGE(TAG,"ERROR: Max chunk exceed,abort ");
            myCanFileTransferHandler->existingTransferProcess=false;
        }
        break;
    case ERROR_TRANSMISSSION:
        /* code */
        break;
    case FILENAME:
 


            // Reconstruct the filename from the 7-byte chunks
            for (int j = 1; j < 8; j++) {
                if (buf[j] == 0) {
                    break; // Stop if the data is null
                }
                outputFilename[indeX++] = buf[j];
            }
        

    outputFilename[indeX] = '\0'; // Add the string termination character

    // Combine the directory and filename
    char fullOutputFilename[100];
    snprintf(fullOutputFilename, sizeof(fullOutputFilename), "%s%s", outputDirectory, outputFilename);
    printf("Output Filename: %s\n", fullOutputFilename);
        break;
    case FILE_TRANSFER_INFO : 

        //chunk sending starded
        if(buf[2]==0x00){

                 if(buf[1]==0x00){
                    //if self (aka esp2 service)
                      printf("Update self started");
                      processUpdateState(0,0);
                }
                if(buf[1]==0x01){
                    //if self (aka esp2 service)
                     printf("Update display started");
                     processUpdateState(1,0);
                }
                if(buf[1]==0x02){
                    //if self (aka esp2 service)
                    printf("Update driver started");
                    processUpdateState(2,0);

                }
        }
        //chunk  chunk finiched 
        else if(buf[2]==0x01){
                    if(buf[1]==0x00){
                    //if self (aka esp2 service)
                       printf("Update self finished");
                       processUpdateState(0,1);
                }
                if(buf[1]==0x01){
                    //if self (aka esp2 service)
                   printf("Update Display finished");
                   processUpdateState(1,1);

                }
                if(buf[1]==0x02){
                    //if self (aka esp2 service)
                    printf("Update driver finished");
                    processUpdateState(2,1);

                }
                if(buf[1]==0x02){
                    //if self (aka esp2 service)
                    printf("All devices Updated");
                    processUpdateState(3,1);

                }

        }
        break;
    default:
        break;
    }
    
    }
   
};

//CANBUS SEND  MESSAGES TO CANBUS
int waitResponse(int timeout){
    int startTime =  esp_timer_get_time();
  
    while (1) {

        if (TWAI_get() == ESP_OK) {
            //uint8_t ret = command_receiver();
            //printf("ret = %d\n", ret);
            //if(idx == UNIVERSAL_ID) {}

            int messageResult = analyzeMessagesFileSender(message);
            ESP_LOGI(TAG,"waiting %d ",messageResult);
            if (messageResult == 1) {
                return 1; // Successful confirmation , pass to next chunk
            } else if (messageResult == 2) {
                return 2; // CRC check failed, resend the current chunk
            } else {
                CAN_resendAction();
            }
            
        }
        // Task code here
        //ESP_LOGI("CAN_BUS_TAsk", "Running...");
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG,"No response so timeout ");
    //twai_clear_transmit_queue();
    //twai_clear_receive_queue();

    return 0;

}

bool isExistingTransferProcess(){
    return myCanFileTransferHandler->existingTransferProcess;
}

esp_err_t createCANReceiverTask() {

    if( xTaskCreate(CAN_FT_messageListner, "messages_receiver_task", 4096*2, NULL, configMAX_PRIORITIES-1, myCanFileTransferHandler->receiverTaskHandle) == pdPASS) {
        ESP_LOGI(TAG,"TASK CREATED");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG,"TASK receiver not created! file will not be  received");
        return 1;
    }
    return 1;

}

esp_err_t deleteCANReceiverTask(){
    vTaskDelete(myCanFileTransferHandler->receiverTaskHandle);

    myCanFileTransferHandler->receiverTaskHandle=NULL;
    return ESP_OK;
}

uint8_t analyzeMessagesFileSender(twai_message_t message){


    if(message.identifier!=0){

        printf("ID: %#lx ", (unsigned long)message.identifier);

    // uint32_t identifier = message.identifier;
        uint8_t priority = (message.identifier >> 8) & 0x7; // Extract the most significant byte (bits 24-26)
        uint8_t source = (message.identifier >> 4) & 0xF;   // Extract the second byte (bits 16-19)
        uint8_t dest = message.identifier  & 0xF;          // Extract the third byte (bits 0-3)

    //printf("ID: %#lx [ ", (unsigned long)message.identifier);
                    /*uint8_t buf[LEN];   
                if (!(message.flags & TWAI_MSG_FLAG_RTR)) {
                    for (int i = 0; i < message.data_length_code; i++) {
                        printf("%02x ", message.data[i]);
                        buf[i] = message.data[i];
                    }*/
            //  printf("  canbus.cpp]\n");
            //ESP_LOGI(TAG,"priority:  %d",priority);
            // ESP_LOGI(TAG,"source:  %d",source);
            // ESP_LOGI(TAG,"dest:  %d",dest);
                    
        //}
        if (message.identifier == 0x84)
        {
        switch (buf[0])
        {
        case 0:
            // Abort file transfert
            return 0;
            break;
        case 1:
            // CRC accepted, next chunk
            return 1;
            break;
        case 2:
            // RESEND chunk
            return 2;
            break;
        
        default:
            return -1;
            
        }
        
        }
         if (message.identifier == RESTART_PROCESSOR)
        {
            ESP_LOGI(TAG, " RESTART ESP...");
            vTaskDelay(5);
            esp_restart();
        }  

        
        return -1;
    }
return -1;
}

void CAN_sendChunk(uint8_t  id,uint8_t * chunk){

    CAN_FT_sendMessage(id,chunk);
}

void CAN_endChunk(uint8_t status)
{
    uint8_t buffer[8] = {0};
     buffer[0] = status;

    CAN_FT_sendMessage(0x89,buffer);
}

void CAN_sendFileNotification(uint8_t dest){
    //TO DO send to the destination file name 
    uint8_t buffer[8] = {0};
    // Extracting each 8-bit segment from the 32-bit integer
    buffer[0] = dest;

    CAN_FT_sendMessage(0x86,buffer);
}

void CAN_sendAction(uint8_t action){
        uint8_t buffer[8] = {0};
    // Extracting each 8-bit segment from the 32-bit integer
    buffer[0] = action; //0 :abort,1:confirmaneand pass to next chunk. 2: crc missmatch resend 

    CAN_FT_sendMessage(ACTION,buffer);
}

void CAN_initChunkTransmission(int32_t totalChunkNbr, uint32_t filesize)
{
    uint8_t buffer[8] = {0};
    // Extracting each 8-bit segment from the 32-bit integer
    buffer[0] = (totalChunkNbr >> 24) & 0xFF; // Most significant byte
    buffer[1] = (totalChunkNbr >> 16) & 0xFF;
    buffer[2] = (totalChunkNbr >> 8) & 0xFF;
    buffer[3] = totalChunkNbr & 0xFF; // Least significant byte

     // Extracting each 8-bit segment from the 32-bit filesize
    buffer[4] = (filesize >> 24) & 0xFF; // Most significant byte of filesize
    buffer[5] = (filesize >> 16) & 0xFF;
    buffer[6] = (filesize >> 8) & 0xFF;
    buffer[7] = filesize & 0xFF; // Least significant byte of filesize
    CAN_FT_sendMessage(0x80,buffer);
}

void CAN_sendError(uint8_t errortype){
    uint8_t buffer[8] = {0};

}

void CAN_sendFileName(uint8_t *data){
         CAN_FT_sendMessage(FILENAME,data);
}

void CAN_sendFileTransfertInfo(uint8_t *device,uint8_t status){
    uint8_t buffer[8] = {0};
    buffer[0] = 0x07;
    buffer[1] = device;
    buffer[2] =status;
   
    CAN_FT_sendMessage(FILE_TRANSFER_INFO,buffer);
}

void CAN_resendAction() {
    uint8_t buffer[8] = {0};
    CAN_FT_sendMessage(RESEND_ACTION,buffer);
}

esp_err_t divideFileIntoChunks(const char *filename) {
    //const char *TAG = "FILE_DIVIDER";
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return ESP_ERR_NOT_FOUND;
    }

    myCanFileTransferHandler->existingTransferProcess = true;
    senFilename(DRIVER_NODE, "firmware.bin");

    // Get the file size
    fseek(file, 0L, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    unsigned char page[DATA_SIZE * 32];  // 32 chunks per page
    size_t bytesRead;
    size_t totalBytesRead = 0;

    int pageNumber = 0;
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    int estimatedPageNbr = (fileSize + DATA_SIZE * 32 - 1) / (DATA_SIZE * 32);  // Round up division

    CAN_initChunkTransmission(estimatedPageNbr, fileSize);
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    int resendAttempts = 0;
    uint8_t pageCrc = 0;

    while ((bytesRead = fread(page, 1, DATA_SIZE * 32 - 1, file)) > 0) {
        
        totalBytesRead += bytesRead;
        printf("Original size %ld, Remaining file size: %ld\n", fileSize, fileSize - totalBytesRead);

        if(bytesRead<DATA_SIZE * 32 - 1) {
            ESP_LOGW(TAG,"bytesRead < 7, overwrite the remaning bytes to 0 with bytesRead=%d ",bytesRead);
            for (size_t i = bytesRead+1; i < DATA_SIZE * 32 - 1; ++i) {
                ESP_LOGW(TAG,"add 0 at %d",i);
                page[i]=0;
            }
        }
         // Calculate CRC for the entire page
        pageCrc = esp_crc8_le(0, page, 255);

        // Append the CRC to the last byte of the last chunk
        page[255] = pageCrc;
        printf("CRC page %d \n",pageCrc);
        printf("Page %d: ", pageNumber);
        // Send the entire page (8 bytes at a time)
        for (size_t i = 0; i < bytesRead + 1; i += DATA_SIZE) {
            //unsigned char chunk[DATA_SIZE];
            //memcpy(chunk, &page[i], DATA_SIZE);

           // CAN_sendChunk(CHUNK_TRANSFERT, chunk);  // Use CHUNK_TRANSFER as the chunkType for page data
        }

        // Wait for page confirmation

        for (size_t i = 0; i < 256; ++i) {
            printf("%02X ", page[i]);
        }
        printf("\n");
       
        int response = 0;
        bool twaireinit = false;

        for (int retry = 0; retry < 50; ++retry) {

            /*if(retry==5 && twaireinit ==false){
                ESP_LOGW(TAG, "Twai reinit");
                //TWAI_reinit();
                twai_clear_transmit_queue();
                twai_clear_receive_queue();
                twaireinit= true;
                vTaskDelay(10);
            }*/
            vTaskDelay(10);

            for (size_t i = 0; i < 255 + 1; i += DATA_SIZE) {
                unsigned char chunk[DATA_SIZE];
                memcpy(chunk, &page[i], DATA_SIZE);
                CAN_sendChunk(CHUNK_TRANSFERT, chunk);
                memset(chunk, 0, sizeof(chunk));
            }
            
            vTaskDelay(5);
            int i = 0; 
            bool clearTwai = false;

            while (response == 0) {
                ESP_LOGW(TAG, "here  " );

                response = waitResponse(myCanFileTransferHandler->timeout);
                
                //if(clearTwai== false && i >10){
                    /*
                    twai_clear_transmit_queue();
                    twai_clear_receive_queue();
                    clearTwai= true;
                    */
                    //TWAI_reinit();
                    //ESP_LOGW(TAG, "TWAI REINIT   " );
                vTaskDelay(10);

                CAN_resendAction();
                //}
                //vTaskDelay(2);
            }
            
            if (response == 2) {
                ESP_LOGW(TAG, "Resend current page %d ", retry);
                vTaskDelay(10);
                for (size_t i = 0; i < bytesRead + 1; i += DATA_SIZE) {
                    unsigned char chunk[DATA_SIZE];
                    memcpy(chunk, &page[i], DATA_SIZE);
                    CAN_sendChunk(CHUNK_TRANSFERT, chunk);
                    memset(chunk, 0, sizeof(chunk));
                }
                response =  waitResponse(myCanFileTransferHandler->timeout);
            }

            if (response != 0 && response != 2) {
                ESP_LOGW(TAG, "CONFIRMED  SEND ATTEMPT NBR %d ", retry);
                break;  // Exit the loop if response is not 0
            }

            if (retry==49) {
                fclose(file);
                CAN_endChunk(0);
                myCanFileTransferHandler->existingTransferProcess = false;
                return ESP_ERR_TIMEOUT;
            }

            CAN_resendAction();
        }

        switch (response) {
            
            case 1:
                ESP_LOGI(TAG, "Page confirmation received");
                resendAttempts = 0;  // Reset the resend attempt counter
                memset(page, 0, sizeof(page));
                break;

            case 2:
                /*
                ESP_LOGW(TAG, "Resend current page");
                if (resendAttempts < MAX_RESEND_ATTEMPTS) {
                    resendAttempts++;
                    // Resend the current page
                    for (size_t i = 0; i < bytesRead + 1; i += DATA_SIZE) {
                        unsigned char chunk[DATA_SIZE];
                        memcpy(chunk, &page[i], DATA_SIZE);
                        CAN_sendChunk(CHUNK_TRANSFERT, chunk);
                    }
                    waitResponse(myCanFileTransferHandler->timeout);
                } else {
                    ESP_LOGW(TAG, "Max resend attempts reached for the current page");
                    ESP_LOGI(TAG, "Moving to the next page");
                    CAN_endChunk(0);

                    resendAttempts = 0;  // Reset the resend attempt counter
                }
                */
                break;

            default:
                ESP_LOGE(TAG, "Timeout or abort file transfer");
                fclose(file);
                CAN_endChunk(0);
                myCanFileTransferHandler->existingTransferProcess = false;
                return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(10);

        pageNumber++;
    }

    fclose(file);
    CAN_endChunk(1);
    myCanFileTransferHandler->existingTransferProcess = false;
    return ESP_OK;
}
    
esp_err_t senFilename(uint8_t dest,const char *filename) {
    // Split the string into 7-byte chunks and send each chunk along with the custom value
    int len = strlen(filename);
    int numChunks = len / 7 + (len % 7 != 0); // Calculate the number of chunks needed

    for (int i = 0; i < numChunks; i++) {
        uint8_t chunk[8] = {0}; // Initialize the chunk with zeros

        // Add the custom integer value in the first byte
        if (i == 0) {
            chunk[0] =dest;
        }

        // Add the filename data in the remaining 7 bytes
        for (int j = 0; j < 7 && i * 7 + j < len; j++) {
            chunk[j + 1] = filename[i * 7 + j];
        }

        CAN_sendFileName(chunk);
    }
    return ESP_OK;
}

uint32_t calculateFileCrc32(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t *)malloc(file_size);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 0;
    }

    if (fread(buffer, 1, file_size, file) != file_size) {
        fprintf(stderr, "File reading error\n");
        fclose(file);
        free(buffer);
        return 0;
    }

    fclose(file);

    uint32_t crc_value = 0;
    crc_value = esp_crc32_le(crc_value, buffer, file_size);

    free(buffer);

    return crc_value;
}


