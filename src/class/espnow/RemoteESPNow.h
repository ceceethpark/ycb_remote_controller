#ifndef REMOTE_ESPNOW_H
#define REMOTE_ESPNOW_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// 전송할 데이터 구조체
typedef struct struct_message {
  uint8_t buttonId;
  uint8_t buttonState;
  uint32_t timestamp;
} struct_message;

// 전송 상태 콜백 타입
typedef void (*SendCallback)(bool success);

// 업데이트 콜백 타입 (배터리 레벨, RSSI 값)
typedef void (*UpdateCallback)(uint8_t batteryLevel, int8_t rssi);

// 수신 콜백 타입 (MAC 주소, 데이터, 데이터 길이)
typedef void (*ReceiveCallback)(const uint8_t* mac, const uint8_t* data, int len);

class RemoteESPNow {
public:
    RemoteESPNow();
    
    // 초기화
    bool begin();
    
    // 수신기 설정
    bool setReceiver(const uint8_t* macAddress);
    bool setReceiver(uint8_t mac0, uint8_t mac1, uint8_t mac2, 
                     uint8_t mac3, uint8_t mac4, uint8_t mac5);
    
    // 데이터 전송
    bool sendButtonPress(uint8_t buttonId);
    bool sendButtonRelease(uint8_t buttonId);
    bool sendButtonState(uint8_t buttonId, uint8_t state);
    bool sendData(const struct_message* data);
    
    // 콜백 설정
    void setSendCallback(SendCallback callback);
    void setUpdateCallback(UpdateCallback callback);
    void setReceiveCallback(ReceiveCallback callback);
    
    // RSSI 측정
    int8_t getRSSI();
    
    // 업데이트 (1초 주기로 호출)
    void update();
    
    // MAC 주소 가져오기
    String getMacAddress();
    void printMacAddress();
    
    // 상태 확인
    bool isInitialized();
    bool hasReceiver();
    
    // 통계
    uint32_t getSentCount();
    uint32_t getSuccessCount();
    uint32_t getFailCount();
    void resetStats();
    
private:
    uint8_t receiverMac[6];
    bool initialized;
    bool receiverSet;
    SendCallback sendCallback;
    UpdateCallback updateCallback;
    ReceiveCallback receiveCallback;
    
    // 통계
    uint32_t sentCount;
    uint32_t successCount;
    uint32_t failCount;
    
    // RSSI 및 배터리
    int8_t lastRSSI;
    uint8_t batteryLevel;
    unsigned long lastUpdateTime;
    
    // 정적 콜백 (ESP-NOW는 정적 함수만 허용)
    static RemoteESPNow* instance;
    static void onDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void onDataRecvStatic(const uint8_t *mac, const uint8_t *data, int len);
    
    // 내부 함수
    void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    void onDataRecv(const uint8_t *mac, const uint8_t *data, int len);
};

#endif // REMOTE_ESPNOW_H
