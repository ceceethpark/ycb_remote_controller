#include "YbCar.h"
#include "../lcd/RemoteLCD.h"
#include "../espnow/RemoteESPNow.h"

YbCar::YbCar() {
    pLcd = nullptr;
    pEspNow = nullptr;
    lastUpdateTime = 0;
    
    // 차량 데이터 초기화
    vehicleData.speed = 0;
    vehicleData.direction = 0;  // 정지
    vehicleData.batteryLevel = 0;
    vehicleData.motorTemp = 0;
    vehicleData.motorCurrent = 0;
    vehicleData.fetTemp = 0;
    vehicleData.timestamp = 0;
}

void YbCar::begin(RemoteLCD* lcd, RemoteESPNow* espNow) {
    pLcd = lcd;
    pEspNow = espNow;
    
    printf("YbCar 클래스 초기화 완료\r\n");
}

void YbCar::updateVehicleData(const vehicle_message* data) {
    if (!data) return;
    
    // 차량 데이터 업데이트
    vehicleData.speed = data->speed;
    vehicleData.direction = data->direction;
    vehicleData.batteryLevel = data->batteryLevel;
    vehicleData.motorTemp = data->motorTemp;
    vehicleData.motorCurrent = data->motorCurrent;
    vehicleData.fetTemp = data->fetTemp;
    vehicleData.timestamp = data->timestamp;
    
    lastUpdateTime = millis();
    
    // 디버그 출력
    printf("=== 차량 데이터 수신 ===\r\n");
    printf("속도: %d km/h\r\n", vehicleData.speed);
    printf("방향: %s\r\n", getDirectionString());
    printf("배터리: %d%%\r\n", vehicleData.batteryLevel);
    printf("모터 온도: %d°C\r\n", vehicleData.motorTemp);
    printf("모터 전류: %d mA\r\n", vehicleData.motorCurrent);
    printf("FET 온도: %d°C\r\n", vehicleData.fetTemp);
    
    // LCD 업데이트
    updateDisplay();
}

void YbCar::updateDisplay() {
    if (!pLcd) return;
    
    // LCD에 차량 상태 표시
    pLcd->showVehicleSpeed(vehicleData.speed);
    pLcd->showVehicleDirection(vehicleData.direction);
    pLcd->showVehicleBattery(vehicleData.batteryLevel);
    pLcd->showMotorTemp(vehicleData.motorTemp);
    pLcd->showMotorCurrent(vehicleData.motorCurrent);
    pLcd->showFetTemp(vehicleData.fetTemp);
}

bool YbCar::isConnected() const {
    return (millis() - lastUpdateTime) < CONNECTION_TIMEOUT;
}

const char* YbCar::getDirectionString() const {
    switch (vehicleData.direction) {
        case 0: return "정지";
        case 1: return "전진";
        case 2: return "후진";
        default: return "알수없음";
    }
}
