#ifndef YBCAR_H
#define YBCAR_H

#include <Arduino.h>

// Forward declarations
class RemoteLCD;
class RemoteESPNow;

// 차량 데이터 구조체
struct VehicleData {
    uint8_t speed;              // 속도 (km/h)
    uint8_t direction;          // 0: 정지, 1: 전진, 2: 후진
    uint8_t batteryLevel;       // 배터리 잔량 (%)
    int16_t motorTemp;          // 모터 온도 (°C)
    uint16_t motorCurrent;      // 모터 전류 (mA)
    int16_t fetTemp;            // FET 온도 (°C)
    uint32_t timestamp;         // 타임스탬프
};

// ESP-NOW 수신 데이터 구조체 (차량에서 전송)
typedef struct vehicle_message {
    uint8_t speed;
    uint8_t direction;
    uint8_t batteryLevel;
    int16_t motorTemp;
    uint16_t motorCurrent;
    int16_t fetTemp;
    uint32_t timestamp;
} vehicle_message;

class YbCar {
public:
    YbCar();
    
    // 초기화
    void begin(RemoteLCD* lcd, RemoteESPNow* espNow);
    
    // 차량 데이터 업데이트 (ESP-NOW 수신 콜백에서 호출)
    void updateVehicleData(const vehicle_message* data);
    
    // LCD 업데이트
    void updateDisplay();
    
    // 차량 데이터 접근자
    const VehicleData& getVehicleData() const { return vehicleData; }
    uint8_t getSpeed() const { return vehicleData.speed; }
    uint8_t getDirection() const { return vehicleData.direction; }
    uint8_t getBatteryLevel() const { return vehicleData.batteryLevel; }
    int16_t getMotorTemp() const { return vehicleData.motorTemp; }
    uint16_t getMotorCurrent() const { return vehicleData.motorCurrent; }
    int16_t getFetTemp() const { return vehicleData.fetTemp; }
    
    // 연결 상태
    bool isConnected() const;
    unsigned long getLastUpdateTime() const { return lastUpdateTime; }
    
    // 방향 문자열
    const char* getDirectionString() const;
    
private:
    RemoteLCD* pLcd;
    RemoteESPNow* pEspNow;
    
    VehicleData vehicleData;
    unsigned long lastUpdateTime;
    
    static const unsigned long CONNECTION_TIMEOUT = 3000; // 3초
};

#endif // YBCAR_H
