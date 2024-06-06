//  GPIO TX - RX
#define ESP_1_TX     GPIO_NUM_35
#define ESP_1_RX     GPIO_NUM_36

#define ESP_2_TX     GPIO_NUM_17
#define ESP_2_RX     GPIO_NUM_18

//  --------------------------------------------------------------------------------------  ID

//  MY ID
#define  MY_CANBUS_ID       0x02    //  ESP 1 = 0x01

//  PRIORITY ID
#define ERROR_ID            0x000
#define COMMAND_ID          0x100
#define DATA_ID             0x200
#define PING_ID             0x300

//  NODI MITTENTE ID
#define DEVICE_M_ID           0x000
#define ESP1_M_ID             0x010
#define ESP2_M_ID             0x020
#define ESP3_M_ID             0x030

//  NODI DESTINATARIO ID
#define DEVICE_D_ID           0x000
#define ESP1_D_ID             0x001
#define ESP2_D_ID             0x002
#define ESP3_D_ID             0x003
#define BROADCAST_D_ID        0x004

//  --------------------------------------------------------------------------------------  BYTES

//  BYTE0 ID (PRIORITY ID =     COMMAND_ID e DATA_ID)
#define MSG_MOTOR           0x01
#define MSG_ELETTROVALVOLE  0x02
#define MSG_EMERGENCY       0x03
#define MSG_PEDAL           0x04
#define MSG_FAN             0x05
#define MSG_PID             0x06
#define MSG_UPDATES         0x07
#define MSG_MOTOR_SPEED     0x08
#define MSG_TURTLE          0x09
#define MSG_S1              0x0A
#define MSG_AZ_EM           0x0B
#define MSG_ORE             0x0C
#define MSG_VAL_INIT        0x0D

//  BYTE0 ID (PRIORITY ID =     PING_ID)
#define PING_REQUEST        0x00
#define PING_REPLY          0x01

//  BYTE0 ID (PRIORITY ID =     ERROR_ID)
#define ERROR               0x01

//  BYTE1 ID (BYTE0 =           MSG_MOTOR)
#define MOTOR_OFF           0x00
#define MOTOR_ON            0x01
#define MOTOR_REVERSE       0x02
//#define MOTOR_CFB           0x03
#define MOTOR_TURTLE        0x03
#define MOTOR_CURRENT_MAX   0x04

//  BYTE1 ID (BYTE0 =           MSG_MOTOR_SPEED)
#define SPEED_MIN           0x00
#define SPEED_MAX           0x01
#define SPEED_REV           0x02
#define SPEED_SET           0x03

//  BYTE1 ID (BYTE0 =           MSG_PID)
#define PID_MIN             0x00
#define PID_MAX             0x01
#define PID_SET             0x02
#define PID_KP              0x03
#define PID_KI              0x04
#define PID_KD              0x05
#define PID_OFFSET          0x06
#define PID_OFF             0x07

//  BYTE1 ID (BYTE0 =           MSG_EMERGENCY)
#define PHOTOCELL_DAVANTI   0x00
#define PHOTOCELL_RETRO     0x01
#define FUNGO_DX            0x02
#define FUNGO_SX            0x03
#define SERIE_EMERGENZE     0x04
#define LONG_RESET          0x05

//  BYTE1 ID (BYTE0 =           MSG_PEDAL)
#define MANUALE             0x00
#define PEDALE              0x01

//  BYTE1 ID (BYTE0 =           MSG_TURTLE)
#define TURTLE_OFF          0x00
#define TURTLE_ON           0x01

//  BYTE1 ID (BYTE0 =           MSG_S1)
#define SMART_PEDAL         0x00
#define RELE                0x01
//  BYTE2
#define SMART_PEDAL_OFF     0x00
#define SMART_PEDAL_ON      0x01
#define RELE_OFF            0x00
#define RELE_ON             0x01

//  FLAG - LEN
#define FLAG    TWAI_MSG_FLAG_NONE      //  twai_types.h
#define LEN     8

#define UNIVERSAL_ID 0xff
