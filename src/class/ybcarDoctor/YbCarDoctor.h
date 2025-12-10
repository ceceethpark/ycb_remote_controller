#ifndef YBCAR_DOCTOR_H
#define YBCAR_DOCTOR_H

#include <Arduino.h>

// Forward declarations
class RemoteLCD;
class RemoteESPNow;

// 차량 설정 데이터 구조체
struct VehicleSettings {
    // 배터리 & 전류 설정
    uint16_t batteryVoltage;        // 배터리 전압 (48V = 4800)
    uint16_t limitCurrent;          // 최대허용 전류 (200A = 20000)
    
    // 온도 임계값
    int16_t limitMotorTemp;         // 최대허용 모터 온도 (90°C)
    int16_t limitFetTemp;           // 최대 허용 FET온도 (85°C)
    
    // 배터리 전압
    uint16_t lowBattery;            // 배터리 Low Voltage 알람 (23V = 2300)
    
    // 바퀴 설정
    uint8_t barityIm;               // 0: 무한궤도 1:바퀴
    
    // 모터 극성
    uint8_t motor1Polarity;         // 0: 정회전 1:역회전
    uint8_t motor2Polarity;         // 0: 정회전 1:역회전
    
    // 스로틀 설정
    uint16_t throttleOffset;        // 스로틀 오프셋 (300)
    uint16_t throttleInflec;        // 정지시 모터 브레이크 Fast Decay 비율 (900)
    
    // 전후진 설정
    uint8_t forward;                // 전진값 적용비율 (100%)
    uint8_t backward;               // 후진 적용 비율 (80%)
    
    // 가감속 설정
    uint8_t accel;                  // 출발시 가속 상수 (20) 0-100
    uint8_t decel;                  // 정지시 감속 상수 (20) 0-100
    
    // 브레이크 설정
    uint16_t brakeDelay;            // 선차브레이크 지연 시간 (100)
    uint8_t brakeRate;              // 정지시 감속 상수 (STOP신호에 대응) (10)
    
    uint32_t timestamp;             // 타임스탬프
};

// ESP-NOW 설정 메시지 구조체
typedef struct settings_message {
    uint8_t messageType;            // 메시지 타입 (0: 요청, 1: 응답, 2: 설정)
    VehicleSettings settings;
    uint32_t checksum;              // 체크섬
} settings_message;

// 메시지 타입
enum MessageType {
    MSG_REQUEST_SETTINGS = 0,       // 설정 요청
    MSG_RESPONSE_SETTINGS = 1,      // 설정 응답
    MSG_UPDATE_SETTINGS = 2         // 설정 업데이트
};

class YbCarDoctor {
public:
    YbCarDoctor();
    
    // 초기화
    void begin(RemoteLCD* lcd, RemoteESPNow* espNow);
    
    // 설정 요청
    void requestSettings();
    
    // 설정 업데이트 (차량으로 전송)
    bool updateSettings(const VehicleSettings& settings);
    
    // 설정 수신 처리 (ESP-NOW 콜백에서 호출)
    void handleSettingsMessage(const settings_message* msg);
    void handleSettingsMessage(const uint8_t* data, uint8_t len);
    
    // CAN 버퍼에서 설정 로드 (64바이트)
    void loadConfigFromBuffer(const uint8_t* buffer, uint8_t len);
    
    // 현재 설정 가져오기
    const VehicleSettings& getSettings() const { return currentSettings; }
    VehicleSettings& getSettingsRef() { return currentSettings; }
    
    // 설정 값 변경
    void setBatteryVoltage(uint16_t voltage);
    void setLimitCurrent(uint16_t current);
    void setLimitMotorTemp(int16_t temp);
    void setLimitFetTemp(int16_t temp);
    void setLowBattery(uint16_t voltage);
    void setBarityIm(uint8_t mode);
    void setMotor1Polarity(uint8_t polarity);
    void setMotor2Polarity(uint8_t polarity);
    void setThrottleOffset(uint16_t offset);
    void setThrottleInflec(uint16_t inflec);
    void setForward(uint8_t percentage);
    void setBackward(uint8_t percentage);
    void setAccel(uint8_t value);
    void setDecel(uint8_t value);
    void setBrakeDelay(uint16_t delay);
    void setBrakeRate(uint8_t rate);
    
    // 설정 저장/로드 (EEPROM 또는 Preferences)
    bool saveSettings();
    bool loadSettings();
    
    // 설정 화면 표시
    void displaySettings();
    void displaySettingsMenu(uint8_t selectedIndex);
    
    // 상태
    bool isSettingsReceived() const { return settingsReceived; }
    unsigned long getLastUpdateTime() const { return lastUpdateTime; }
    
private:
    RemoteLCD* pLcd;
    RemoteESPNow* pEspNow;
    
    VehicleSettings currentSettings;
    VehicleSettings defaultSettings;
    
    bool settingsReceived;
    unsigned long lastUpdateTime;
    unsigned long lastRequestTime;
    
    // 내부 함수
    uint32_t calculateChecksum(const VehicleSettings& settings);
    bool verifyChecksum(const settings_message* msg);
    void initializeDefaultSettings();
    void applySettings(const VehicleSettings& settings);
};

#endif // YBCAR_DOCTOR_H
