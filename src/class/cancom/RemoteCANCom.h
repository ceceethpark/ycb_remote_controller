#ifndef REMOTE_CANCOM_H
#define REMOTE_CANCOM_H

#include <Arduino.h>
#include <driver/twai.h>

// Forward declarations
class RemoteLCD;
class YbCarDoctor;

// CAN 메시지 ID (차량 실제 프로토콜)
enum CANMessageType {
    CAN_TX_GET_CONFIG = 0x700,      // Config Read Command
    CAN_TX_PUT_CMD = 0x701,         // Config Writer Command  
    CAN_TX_SAVE_CMD = 0x708,        // CAN SaveConfigEvent (0x708~0x70F)
    CAN_RX_DATA_ID_BASE = 0x5B0,    // CAN_RetrieveConfigEvent (0x5B0~0x5B7, 8개 메시지)
    CAN_RX_RESPONSE_ID = 0x5B8      // CAN_From_VCU
};

// 비트필드 커맨드 구조체
typedef struct {
    uint8_t retrieve : 1;  // Config 읽기 요청
    uint8_t saving : 1;    // Config 저장 요청
    uint8_t x2 : 1;
    uint8_t x3 : 1;
    uint8_t x4 : 1;
    uint8_t x5 : 1;
    uint8_t x6 : 1;
    uint8_t x7 : 1;
} BIT_COMMAND;

typedef union {
    uint8_t u8;
    BIT_COMMAND b;
} TX_COMMAND_BYTE;

class RemoteCANCom {
public:
    RemoteCANCom();
    
    // 초기화
    bool begin(gpio_num_t txPin = GPIO_NUM_21, gpio_num_t rxPin = GPIO_NUM_22);
    
    // CAN 통신
    bool sendMessage(uint32_t id, const uint8_t* data, uint8_t len);
    bool receiveMessage(twai_message_t& message);
    bool isMessageAvailable();
    
    // 설정 모드
    bool enterSettingsMode();  // SELECT+LEFT+RIGHT 조합으로 진입
    bool exitSettingsMode();
    bool isInSettingsMode();
    
    // 설정 데이터 송수신 (실제 프로토콜)
    bool requestConfig();      // 0x700: Config Read (retrieve=1)
    bool saveConfig();         // 0x708: Config Save
    bool updateConfig(uint8_t index, const uint8_t* data, uint8_t len);  // 0x701: Config Write
    bool parseConfigData(uint32_t canId, const uint8_t* data, uint8_t len);  // 0x5B0~0x5B7 파싱
    
    // 설정 데이터 버퍼 (8개 메시지 x 8바이트 = 64바이트)
    uint8_t configBuffer[64];
    uint8_t getBufferIndex() const { return bufferIndex; }
    bool isConfigComplete() const { return bufferIndex >= 8; }
    
    // 핸들러 설정
    void setHandlers(RemoteLCD* lcd, YbCarDoctor* doctor);
    
    // 업데이트 (loop에서 호출)
    void update();
    
    // 콜백 설정
    void setReceiveCallback(void (*callback)(const twai_message_t&));
    
    // 상수
    static const gpio_num_t CAN_TX_PIN = GPIO_NUM_21;
    static const gpio_num_t CAN_RX_PIN = GPIO_NUM_22;
    
private:
    bool initialized;
    
    RemoteLCD* pLcd;
    YbCarDoctor* pDoctor;
    
    bool settingsMode;
    unsigned long lastMessageTime;
    
    void (*receiveCallback)(const twai_message_t&);
    
    // 설정 데이터 버퍼 인덱스
    uint8_t bufferIndex;
    
    // 내부 처리 함수
    void processReceivedMessage(const twai_message_t& message);
    void handleConfigDataMessage(uint32_t canId, const twai_message_t& message);
    void handleResponseMessage(const twai_message_t& message);
};

#endif // REMOTE_CANCOM_H
