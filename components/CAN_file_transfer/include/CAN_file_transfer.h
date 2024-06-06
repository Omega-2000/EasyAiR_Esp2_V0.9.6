#ifndef CAN_FILE_TRANSFER_H
#define CAN_FILE_TRANSFER_H
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <driver/twai.h>
#include "TWAI.h"
#define delay(ms) vTaskDelay(ms/portTICK_PERIOD_MS)

/*Destination node*/
#define DEVICE_TEST_NODE 0x000
#define SELF_NODE        0x001
#define ESP2_NODE        0x002
#define DRIVER_NODE      0x003
#define ALL_NODES        0x004 //broadcast

//DEFINE ID 
#define INIT_TRANSMISSION 0x80
#define END_TRANSMISSION  0x81
#define CHUNK_TRANSFERT   0x82
#define ERROR_TRANSMISSSION 0x83
#define ACTION  0x84
#define FILENAME  0x85
#define FILE_TRANSFER_INFO  0x86
#define RESEND_ACTION  0x87  // Ask receiver to resend Action (next page or abort etc..)
#define RESTART_PROCESSOR 0x91


#define DATA_SIZE 8 // 7 bytes of data and 1 byte for CRC
#define MAX_CHUNK_SIZE 8
#define MAX_RESEND_ATTEMPTS 3

typedef enum {
    SENDER_MODE,
    RECEIVER_MODE
} TransferMode;

 struct CANFileTransfer{
    int chunkCount;
    int MaxChunk;
    FILE *outputFile;
    long fileSize;
    uint32_t expectedFileSize;
    const char *outputFilename;
    uint8_t canbusSourceID;
    TransferMode mode;
    bool existingTransferProcess;
    unsigned int timeout;
    TaskHandle_t receiverTaskHandle;
} ;

#ifdef __cplusplus
extern "C" {
#endif


struct CANFileTransfer * createNewCanFileTransfer();
void CAN_FT_messageListner(void *pvParameters);
int CAN_FT_initCanbus();
void CAN_FT_sendMessage(uint32_t id, uint8_t data[]);
void checkSpiffsFreeSpace();
void analyzeMessagesFileReceiver(twai_message_t message);
uint8_t analyzeMessagesFileSender(twai_message_t message);
int waitResponse(int timeout);
bool isExistingTransferProcess();
esp_err_t createCANReceiverTask();
esp_err_t deleteCANReceiverTask();
void CAN_sendChunk(uint8_t id, uint8_t *chunk);
void CAN_endChunk(uint8_t status);
void CAN_sendFileNotification(uint8_t dest);
void CAN_sendAction(uint8_t action);
void CAN_initChunkTransmission(int32_t totalChunkNbr, uint32_t filesize);
void CAN_sendFileName(uint8_t *data);
void CAN_sendFileTransfertInfo(uint8_t *device,uint8_t status);

void CAN_resendAction();
esp_err_t divideFileIntoChunks(const char *filename);
esp_err_t senFilename(uint8_t dest,const char *filename);
uint32_t calculateFileCrc32(const char *filename);

//callback function to pass state (star/end) of device update
typedef void (*UpdateStateChangedHandler)(int device, int state);
void registerUpdateStateChangedHandler(UpdateStateChangedHandler handler);
void processUpdateState(int device, int state);






#ifdef __cplusplus
}
#endif
#endif