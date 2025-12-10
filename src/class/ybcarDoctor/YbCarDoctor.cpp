#include "YbCarDoctor.h"
#include "../lcd/RemoteLCD.h"
#include "../espnow/RemoteESPNow.h"
#include <Preferences.h>

Preferences preferences;

YbCarDoctor::YbCarDoctor() {
    pLcd = nullptr;
    pEspNow = nullptr;
    settingsReceived = false;
    lastUpdateTime = 0;
    lastRequestTime = 0;
    
    // 기본 설정 초기화
    initializeDefaultSettings();
    currentSettings = defaultSettings;
}

void YbCarDoctor::begin(RemoteLCD* lcd, RemoteESPNow* espNow) {
    pLcd = lcd;
    pEspNow = espNow;
    
    // 저장된 설정 로드 시도
    if (!loadSettings()) {
        printf("저장된 설정 없음 - 기본값 사용\r\n");
        currentSettings = defaultSettings;
    }
    
    printf("YbCarDoctor 클래스 초기화 완료\r\n");
}

void YbCarDoctor::initializeDefaultSettings() {
    // 배터리 & 전류 설정
    defaultSettings.batteryVoltage = 4800;      // 48V
    defaultSettings.limitCurrent = 20000;       // 200A
    
    // 온도 임계값
    defaultSettings.limitMotorTemp = 90;        // 90°C
    defaultSettings.limitFetTemp = 85;          // 85°C
    
    // 배터리 전압
    defaultSettings.lowBattery = 2300;          // 23V
    
    // 바퀴 설정
    defaultSettings.barityIm = 1;               // 1: 바퀴
    
    // 모터 극성
    defaultSettings.motor1Polarity = 0;         // 0: 정회전
    defaultSettings.motor2Polarity = 0;         // 0: 정회전
    
    // 스로틀 설정
    defaultSettings.throttleOffset = 300;       // 스로틀 오프셋
    defaultSettings.throttleInflec = 900;       // 브레이크 Fast Decay
    
    // 전후진 설정
    defaultSettings.forward = 100;              // 전진 100%
    defaultSettings.backward = 80;              // 후진 80%
    
    // 가감속 설정
    defaultSettings.accel = 20;                 // 가속 상수
    defaultSettings.decel = 20;                 // 감속 상수
    
    // 브레이크 설정
    defaultSettings.brakeDelay = 100;           // 브레이크 지연
    defaultSettings.brakeRate = 10;             // 브레이크 감속 상수
    
    defaultSettings.timestamp = millis();
}

void YbCarDoctor::requestSettings() {
    if (!pEspNow) return;
    
    // 중복 요청 방지 (1초 이내 재요청 불가)
    if (millis() - lastRequestTime < 1000) {
        return;
    }
    
    settings_message msg;
    msg.messageType = MSG_REQUEST_SETTINGS;
    msg.settings = currentSettings;  // 현재 설정을 담아 보냄
    msg.checksum = calculateChecksum(currentSettings);
    
    // ESP-NOW로 전송
    esp_err_t result = esp_now_send(nullptr, (uint8_t*)&msg, sizeof(msg));
    
    if (result == ESP_OK) {
        printf("설정 요청 전송 완료\r\n");
        lastRequestTime = millis();
    } else {
        printf("설정 요청 전송 실패!\r\n");
    }
}

bool YbCarDoctor::updateSettings(const VehicleSettings& settings) {
    if (!pEspNow) return false;
    
    settings_message msg;
    msg.messageType = MSG_UPDATE_SETTINGS;
    msg.settings = settings;
    msg.checksum = calculateChecksum(settings);
    
    // ESP-NOW로 전송
    esp_err_t result = esp_now_send(nullptr, (uint8_t*)&msg, sizeof(msg));
    
    if (result == ESP_OK) {
        printf("=== 설정 업데이트 전송 ===\r\n");
        printf("배터리: %dV\r\n", settings.batteryVoltage / 100);
        printf("최대전류: %dA\r\n", settings.limitCurrent / 100);
        printf("전진: %d%%, 후진: %d%%\r\n", settings.forward, settings.backward);
        printf("모터온도: %dC, FET온도: %dC\r\n", settings.limitMotorTemp, settings.limitFetTemp);
        
        currentSettings = settings;
        lastUpdateTime = millis();
        return true;
    } else {
        printf("설정 업데이트 전송 실패!\r\n");
        return false;
    }
}

void YbCarDoctor::handleSettingsMessage(const settings_message* msg) {
    if (!msg) return;
    
    // 체크섬 검증
    if (!verifyChecksum(msg)) {
        printf("설정 메시지 체크섬 오류!\r\n");
        return;
    }
    
    switch (msg->messageType) {
        case MSG_REQUEST_SETTINGS:
            printf("설정 요청 수신 (차량에서)\r\n");
            // 차량에서 설정을 요청 - 현재 설정을 응답
            updateSettings(currentSettings);
            break;
            
        case MSG_RESPONSE_SETTINGS:
            printf("=== 설정 응답 수신 ===\r\n");
            currentSettings = msg->settings;
            settingsReceived = true;
            lastUpdateTime = millis();
            
            // 설정 정보 출력
            printf("배터리전압: %d V\r\n", currentSettings.batteryVoltage / 100);
            printf("최대전류: %d A\r\n", currentSettings.limitCurrent / 100);
            printf("모터온도제한: %d°C\r\n", currentSettings.limitMotorTemp);
            printf("FET온도제한: %d°C\r\n", currentSettings.limitFetTemp);
            printf("전진: %d%%, 후진: %d%%\r\n", currentSettings.forward, currentSettings.backward);
            
            // LCD에 표시
            displaySettings();
            break;
            
        case MSG_UPDATE_SETTINGS:
            printf("설정 업데이트 확인 수신\r\n");
            settingsReceived = true;
            lastUpdateTime = millis();
            break;
            
        default:
            printf("알 수 없는 메시지 타입\r\n");
            break;
    }
}

uint32_t YbCarDoctor::calculateChecksum(const VehicleSettings& settings) {
    uint32_t checksum = 0;
    const uint8_t* data = (const uint8_t*)&settings;
    for (size_t i = 0; i < sizeof(VehicleSettings); i++) {
        checksum += data[i];
    }
    return checksum;
}

bool YbCarDoctor::verifyChecksum(const settings_message* msg) {
    uint32_t calculated = calculateChecksum(msg->settings);
    return (calculated == msg->checksum);
}

void YbCarDoctor::applySettings(const VehicleSettings& settings) {
    currentSettings = settings;
    currentSettings.timestamp = millis();
}

// 설정 값 변경 함수들
void YbCarDoctor::setBatteryVoltage(uint16_t voltage) {
    currentSettings.batteryVoltage = voltage;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setLimitCurrent(uint16_t current) {
    currentSettings.limitCurrent = current;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setLimitMotorTemp(int16_t temp) {
    currentSettings.limitMotorTemp = temp;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setLimitFetTemp(int16_t temp) {
    currentSettings.limitFetTemp = temp;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setLowBattery(uint16_t voltage) {
    currentSettings.lowBattery = voltage;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setBarityIm(uint8_t mode) {
    currentSettings.barityIm = mode;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setMotor1Polarity(uint8_t polarity) {
    currentSettings.motor1Polarity = polarity;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setMotor2Polarity(uint8_t polarity) {
    currentSettings.motor2Polarity = polarity;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setThrottleOffset(uint16_t offset) {
    currentSettings.throttleOffset = offset;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setThrottleInflec(uint16_t inflec) {
    currentSettings.throttleInflec = inflec;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setForward(uint8_t percentage) {
    currentSettings.forward = percentage;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setBackward(uint8_t percentage) {
    currentSettings.backward = percentage;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setAccel(uint8_t value) {
    currentSettings.accel = value;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setDecel(uint8_t value) {
    currentSettings.decel = value;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setBrakeDelay(uint16_t delay) {
    currentSettings.brakeDelay = delay;
    currentSettings.timestamp = millis();
}

void YbCarDoctor::setBrakeRate(uint8_t rate) {
    currentSettings.brakeRate = rate;
    currentSettings.timestamp = millis();
}

bool YbCarDoctor::saveSettings() {
    preferences.begin("ybcar", false);
    
    bool success = true;
    success &= preferences.putUShort("battV", currentSettings.batteryVoltage);
    success &= preferences.putUShort("limitI", currentSettings.limitCurrent);
    success &= preferences.putShort("limitMT", currentSettings.limitMotorTemp);
    success &= preferences.putShort("limitFT", currentSettings.limitFetTemp);
    success &= preferences.putUShort("lowBatt", currentSettings.lowBattery);
    success &= preferences.putUChar("barity", currentSettings.barityIm);
    success &= preferences.putUChar("m1pol", currentSettings.motor1Polarity);
    success &= preferences.putUChar("m2pol", currentSettings.motor2Polarity);
    success &= preferences.putUShort("thrOff", currentSettings.throttleOffset);
    success &= preferences.putUShort("thrInf", currentSettings.throttleInflec);
    success &= preferences.putUChar("forward", currentSettings.forward);
    success &= preferences.putUChar("backward", currentSettings.backward);
    success &= preferences.putUChar("accel", currentSettings.accel);
    success &= preferences.putUChar("decel", currentSettings.decel);
    success &= preferences.putUShort("brkDel", currentSettings.brakeDelay);
    success &= preferences.putUChar("brkRate", currentSettings.brakeRate);
    
    preferences.end();
    
    if (success) {
        printf("설정 저장 완료\r\n");
    } else {
        printf("설정 저장 실패!\r\n");
    }
    
    return success;
}

bool YbCarDoctor::loadSettings() {
    preferences.begin("ybcar", true);  // read-only
    
    // 저장된 설정이 있는지 확인
    if (!preferences.isKey("battV")) {
        preferences.end();
        return false;
    }
    
    currentSettings.batteryVoltage = preferences.getUShort("battV", 4800);
    currentSettings.limitCurrent = preferences.getUShort("limitI", 20000);
    currentSettings.limitMotorTemp = preferences.getShort("limitMT", 90);
    currentSettings.limitFetTemp = preferences.getShort("limitFT", 85);
    currentSettings.lowBattery = preferences.getUShort("lowBatt", 2300);
    currentSettings.barityIm = preferences.getUChar("barity", 1);
    currentSettings.motor1Polarity = preferences.getUChar("m1pol", 0);
    currentSettings.motor2Polarity = preferences.getUChar("m2pol", 0);
    currentSettings.throttleOffset = preferences.getUShort("thrOff", 300);
    currentSettings.throttleInflec = preferences.getUShort("thrInf", 900);
    currentSettings.forward = preferences.getUChar("forward", 100);
    currentSettings.backward = preferences.getUChar("backward", 80);
    currentSettings.accel = preferences.getUChar("accel", 20);
    currentSettings.decel = preferences.getUChar("decel", 20);
    currentSettings.brakeDelay = preferences.getUShort("brkDel", 100);
    currentSettings.brakeRate = preferences.getUChar("brkRate", 10);
    currentSettings.timestamp = millis();
    
    preferences.end();
    
    printf("저장된 설정 로드 완료\r\n");
    return true;
}

void YbCarDoctor::displaySettings() {
    if (!pLcd) return;
    
    pLcd->clear();
    pLcd->setTextSize(2);
    pLcd->printTextCentered("차량 설정", 5, RemoteLCD::CYAN);
    
    pLcd->setTextSize(1);
    char text[35];
    
    // 배터리 & 전류
    sprintf(text, "배터리: %dV", currentSettings.batteryVoltage / 100);
    pLcd->printText(text, 10, 30, RemoteLCD::WHITE);
    
    sprintf(text, "최대전류: %dA", currentSettings.limitCurrent / 100);
    pLcd->printText(text, 10, 45, RemoteLCD::WHITE);
    
    // 온도 제한
    sprintf(text, "모터온도: %dC", currentSettings.limitMotorTemp);
    pLcd->printText(text, 10, 60, RemoteLCD::YELLOW);
    
    sprintf(text, "FET온도: %dC", currentSettings.limitFetTemp);
    pLcd->printText(text, 10, 75, RemoteLCD::YELLOW);
    
    // 저전압
    sprintf(text, "저전압: %dV", currentSettings.lowBattery / 100);
    pLcd->printText(text, 10, 90, RemoteLCD::RED);
    
    // 바퀴타입
    sprintf(text, "타입: %s", currentSettings.barityIm ? "바퀴" : "무한궤도");
    pLcd->printText(text, 10, 105, RemoteLCD::CYAN);
    
    // 모터 극성
    sprintf(text, "M1:%s M2:%s", 
            currentSettings.motor1Polarity ? "역" : "정",
            currentSettings.motor2Polarity ? "역" : "정");
    pLcd->printText(text, 10, 120, RemoteLCD::MAGENTA);
    
    // 스로틀
    sprintf(text, "스로틀 Ofs:%d Inf:%d", 
            currentSettings.throttleOffset, currentSettings.throttleInflec);
    pLcd->printText(text, 10, 135, RemoteLCD::WHITE);
    
    // 전후진
    sprintf(text, "전진:%d%% 후진:%d%%", 
            currentSettings.forward, currentSettings.backward);
    pLcd->printText(text, 10, 150, RemoteLCD::GREEN);
    
    // 가감속
    sprintf(text, "가속:%d 감속:%d", 
            currentSettings.accel, currentSettings.decel);
    pLcd->printText(text, 10, 165, RemoteLCD::YELLOW);
    
    // 브레이크
    sprintf(text, "브레이크 Dly:%d Rate:%d", 
            currentSettings.brakeDelay, currentSettings.brakeRate);
    pLcd->printText(text, 10, 180, RemoteLCD::RED);
}

void YbCarDoctor::displaySettingsMenu(uint8_t selectedIndex) {
    if (!pLcd) return;
    
    pLcd->clear();
    pLcd->setTextSize(2);
    pLcd->printTextCentered("설정 메뉴", 5, RemoteLCD::CYAN);
    
    pLcd->setTextSize(1);
    
    const char* menuItems[] = {
        "1. 최대속도",
        "2. 최대전류",
        "3. 가속도",
        "4. 브레이크",
        "5. 온도제한",
        "6. 배터리",
        "7. 트림설정",
        "8. 시스템"
    };
    
    for (uint8_t i = 0; i < 8; i++) {
        uint16_t color = (i == selectedIndex) ? RemoteLCD::YELLOW : RemoteLCD::WHITE;
        uint16_t yPos = 30 + (i * 25);
        
        if (i == selectedIndex) {
            pLcd->printText(">", 5, yPos, RemoteLCD::GREEN);
        }
        
        pLcd->printText(menuItems[i], 20, yPos, color);
    }
}

// CAN 버퍼에서 설정 로드 (64바이트 데이터 파싱)
void YbCarDoctor::loadConfigFromBuffer(const uint8_t* buffer, uint8_t len) {
    if (len < 64) {
        printf("Config 버퍼 길이 부족: %d 바이트 (필요: 64바이트)\r\n", len);
        return;
    }
    
    printf("=== CAN Config 데이터 파싱 시작 ===\r\n");
    
    // 64바이트 데이터를 VehicleSettings 구조체로 파싱
    // 실제 차량 프로토콜에 맞게 바이트 오프셋 조정 필요
    
    uint16_t offset = 0;
    
    // 배터리 전압 (2바이트, Little Endian)
    currentSettings.batteryVoltage = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
    printf("배터리 전압: %d (raw: %d)\r\n", currentSettings.batteryVoltage / 100, currentSettings.batteryVoltage);
    
    // 최대 전류 (2바이트)
    currentSettings.limitCurrent = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
    printf("최대 전류: %d A (raw: %d)\r\n", currentSettings.limitCurrent / 100, currentSettings.limitCurrent);
    
    // 모터 온도 제한 (2바이트, signed)
    currentSettings.limitMotorTemp = (int16_t)(buffer[offset] | (buffer[offset + 1] << 8));
    offset += 2;
    printf("모터 온도 제한: %d°C\r\n", currentSettings.limitMotorTemp);
    
    // FET 온도 제한 (2바이트, signed)
    currentSettings.limitFetTemp = (int16_t)(buffer[offset] | (buffer[offset + 1] << 8));
    offset += 2;
    printf("FET 온도 제한: %d°C\r\n", currentSettings.limitFetTemp);
    
    // 저전압 임계값 (2바이트)
    currentSettings.lowBattery = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
    printf("저전압 임계값: %d V (raw: %d)\r\n", currentSettings.lowBattery / 100, currentSettings.lowBattery);
    
    // 바리티 (1바이트)
    currentSettings.barityIm = buffer[offset++];
    printf("바리티: %d (0:무한궤도, 1:바퀴)\r\n", currentSettings.barityIm);
    
    // 모터1 극성 (1바이트)
    currentSettings.motor1Polarity = buffer[offset++];
    printf("모터1 극성: %d\r\n", currentSettings.motor1Polarity);
    
    // 모터2 극성 (1바이트)
    currentSettings.motor2Polarity = buffer[offset++];
    printf("모터2 극성: %d\r\n", currentSettings.motor2Polarity);
    
    // 스로틀 오프셋 (2바이트)
    currentSettings.throttleOffset = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
    printf("스로틀 오프셋: %d\r\n", currentSettings.throttleOffset);
    
    // 스로틀 변곡점 (2바이트)
    currentSettings.throttleInflec = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
    printf("스로틀 변곡점: %d\r\n", currentSettings.throttleInflec);
    
    // 전진 비율 (1바이트)
    currentSettings.forward = buffer[offset++];
    printf("전진 비율: %d%%\r\n", currentSettings.forward);
    
    // 후진 비율 (1바이트)
    currentSettings.backward = buffer[offset++];
    printf("후진 비율: %d%%\r\n", currentSettings.backward);
    
    // 가속 (1바이트)
    currentSettings.accel = buffer[offset++];
    printf("가속: %d\r\n", currentSettings.accel);
    
    // 감속 (1바이트)
    currentSettings.decel = buffer[offset++];
    printf("감속: %d\r\n", currentSettings.decel);
    
    // 브레이크 지연 (2바이트)
    currentSettings.brakeDelay = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
    printf("브레이크 지연: %d ms\r\n", currentSettings.brakeDelay);
    
    // 브레이크 레이트 (1바이트)
    currentSettings.brakeRate = buffer[offset++];
    printf("브레이크 레이트: %d\r\n", currentSettings.brakeRate);
    
    // 타임스탬프 업데이트
    currentSettings.timestamp = millis();
    
    settingsReceived = true;
    lastUpdateTime = millis();
    
    printf("=== Config 파싱 완료 (사용: %d / 64 바이트) ===\r\n", offset);
    
    // LCD에 설정 표시
    displaySettings();
}

// CAN handleSettingsMessage 오버로드 (바이트 배열)
void YbCarDoctor::handleSettingsMessage(const uint8_t* data, uint8_t len) {
    printf("CAN 설정 메시지 수신: %d 바이트\r\n", len);
    
    // 단순 버퍼 복사 (추후 프로토콜에 따라 수정)
    if (len <= 64) {
        loadConfigFromBuffer(data, len);
    }
}
