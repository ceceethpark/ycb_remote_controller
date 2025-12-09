#include "RemoteESPNow.h"

// 정적 인스턴스 포인터
RemoteESPNow* RemoteESPNow::instance = nullptr;

RemoteESPNow::RemoteESPNow() {
    initialized = false;
    receiverSet = false;
    sendCallback = nullptr;
    updateCallback = nullptr;
    receiveCallback = nullptr;
    sentCount = 0;
    successCount = 0;
    failCount = 0;
    lastRSSI = 0;
    batteryLevel = 100;
    lastUpdateTime = 0;
    
    // 싱글톤 인스턴스 설정
    instance = this;
    
    // 수신기 MAC 초기화 (브로드캠스트)
    for (int i = 0; i < 6; i++) {
        receiverMac[i] = 0xFF;
    }
}

bool RemoteESPNow::begin() {
    // WiFi를 Station 모드로 설정
    WiFi.mode(WIFI_STA);
    
    Serial.println("=== ESP-NOW 초기화 ===");
    Serial.print("MAC 주소: ");
    Serial.println(WiFi.macAddress());
    
    // ESP-NOW 초기화
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW 초기화 실패!");
        initialized = false;
        return false;
    }
    
    Serial.println("ESP-NOW 초기화 성공");
    
    // 전송/수신 콜백 등록
    esp_now_register_send_cb(onDataSentStatic);
    esp_now_register_recv_cb(onDataRecvStatic);
    
    initialized = true;
    return true;
}

bool RemoteESPNow::setReceiver(const uint8_t* macAddress) {
    if (!initialized) {
        Serial.println("ESP-NOW가 초기화되지 않았습니다!");
        return false;
    }
    
    // 기존 피어가 있으면 제거
    if (receiverSet) {
        esp_now_del_peer(receiverMac);
    }
    
    // MAC 주소 복사
    memcpy(receiverMac, macAddress, 6);
    
    // 피어 정보 설정
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiverMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    // 피어 추가
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("수신기 추가 실패!");
        receiverSet = false;
        return false;
    }
    
    Serial.print("수신기 설정 완료: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", receiverMac[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
    
    receiverSet = true;
    return true;
}

bool RemoteESPNow::setReceiver(uint8_t mac0, uint8_t mac1, uint8_t mac2, 
                                uint8_t mac3, uint8_t mac4, uint8_t mac5) {
    uint8_t mac[6] = {mac0, mac1, mac2, mac3, mac4, mac5};
    return setReceiver(mac);
}

bool RemoteESPNow::sendButtonPress(uint8_t buttonId) {
    return sendButtonState(buttonId, 1);
}

bool RemoteESPNow::sendButtonRelease(uint8_t buttonId) {
    return sendButtonState(buttonId, 0);
}

bool RemoteESPNow::sendButtonState(uint8_t buttonId, uint8_t state) {
    struct_message data;
    data.buttonId = buttonId;
    data.buttonState = state;
    data.timestamp = millis();
    
    return sendData(&data);
}

bool RemoteESPNow::sendData(const struct_message* data) {
    if (!initialized) {
        Serial.println("ESP-NOW가 초기화되지 않았습니다!");
        return false;
    }
    
    if (!receiverSet) {
        Serial.println("수신기가 설정되지 않았습니다!");
        return false;
    }
    
    sentCount++;
    
    esp_err_t result = esp_now_send(receiverMac, (uint8_t*)data, sizeof(struct_message));
    
    if (result == ESP_OK) {
        Serial.print("버튼 ");
        Serial.print(data->buttonId);
        Serial.print(" 전송 요청 성공 (상태: ");
        Serial.print(data->buttonState);
        Serial.println(")");
        return true;
    } else {
        Serial.print("버튼 ");
        Serial.print(data->buttonId);
        Serial.println(" 전송 요청 실패!");
        failCount++;
        return false;
    }
}

void RemoteESPNow::setSendCallback(SendCallback callback) {
    sendCallback = callback;
}

String RemoteESPNow::getMacAddress() {
    return WiFi.macAddress();
}

void RemoteESPNow::printMacAddress() {
    Serial.print("리모컨 MAC 주소: ");
    Serial.println(WiFi.macAddress());
}

bool RemoteESPNow::isInitialized() {
    return initialized;
}

bool RemoteESPNow::hasReceiver() {
    return receiverSet;
}

uint32_t RemoteESPNow::getSentCount() {
    return sentCount;
}

uint32_t RemoteESPNow::getSuccessCount() {
    return successCount;
}

uint32_t RemoteESPNow::getFailCount() {
    return failCount;
}

void RemoteESPNow::resetStats() {
    sentCount = 0;
    successCount = 0;
    failCount = 0;
}

void RemoteESPNow::onDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (instance) {
        instance->onDataSent(mac_addr, status);
    }
}

void RemoteESPNow::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    bool success = (status == ESP_NOW_SEND_SUCCESS);
    
    if (success) {
        successCount++;
        Serial.println("→ 전송 성공!");
    } else {
        failCount++;
        Serial.println("→ 전송 실패!");
    }
    
    // 사용자 콜백 호출
    if (sendCallback) {
        sendCallback(success);
    }
}

void RemoteESPNow::setUpdateCallback(UpdateCallback callback) {
    updateCallback = callback;
}

int8_t RemoteESPNow::getRSSI() {
    // WiFi RSSI 값 읽기
    return WiFi.RSSI();
}

void RemoteESPNow::update() {
    // 1초 주기로 업데이트
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= 1000) {
        lastUpdateTime = currentTime;
        
        // RSSI 측정
        lastRSSI = getRSSI();
        
        // 배터리 레벨 측정 (ADC로 실제 전압 측정 필요)
        // 예제: GPIO 34를 사용한다고 가정 (3.3V = 4095, 2.8V = 3482)
        // int adcValue = analogRead(34);
        // batteryLevel = map(constrain(adcValue, 2800, 4095), 2800, 4095, 0, 100);
        
        // 현재는 시뮬레이션 데이터 사용 (서서히 감소)
        static uint8_t simulatedBattery = 100;
        if (currentTime % 60000 < 1000) { // 1분마다 1% 감소
            if (simulatedBattery > 0) simulatedBattery--;
        }
        batteryLevel = simulatedBattery;
        
        // 업데이트 콜백 호출
        if (updateCallback) {
            updateCallback(batteryLevel, lastRSSI);
        }
        
        // 디버그 출력
        Serial.print("업데이트 - 배터리: ");
        Serial.print(batteryLevel);
        Serial.print("%, RSSI: ");
        Serial.print(lastRSSI);
        Serial.println(" dBm");
    }
}

void RemoteESPNow::setReceiveCallback(ReceiveCallback callback) {
    receiveCallback = callback;
}

void RemoteESPNow::onDataRecvStatic(const uint8_t *mac, const uint8_t *data, int len) {
    if (instance) {
        instance->onDataRecv(mac, data, len);
    }
}

void RemoteESPNow::onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
    // 디버그 출력
    Serial.print("데이터 수신 (");
    Serial.print(len);
    Serial.print(" bytes) from: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
    
    // 사용자 콜백 호출
    if (receiveCallback) {
        receiveCallback(mac, data, len);
    }
}
