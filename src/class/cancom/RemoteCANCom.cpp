#include "RemoteCANCom.h"
#include "../lcd/RemoteLCD.h"
#include "../ybcarDoctor/YbCarDoctor.h"

RemoteCANCom::RemoteCANCom() {
    initialized = false;
    pLcd = nullptr;
    pDoctor = nullptr;
    settingsMode = false;
    lastMessageTime = 0;
    receiveCallback = nullptr;
    bufferIndex = 0;
    memset(configBuffer, 0, sizeof(configBuffer));
}

bool RemoteCANCom::begin(gpio_num_t txPin, gpio_num_t rxPin) {
    printf("ESP32 내장 CAN 초기화 시작...\r\n");
    
    // TWAI 일반 설정
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
    
    // TWAI 타이밍 설정 (500 kbps)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    
    // TWAI 필터 설정 (모든 메시지 수신)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    // TWAI 드라이버 설치
    esp_err_t result = twai_driver_install(&g_config, &t_config, &f_config);
    if (result != ESP_OK) {
        printf("CAN 드라이버 설치 실패! 오류: %d\r\n", result);
        return false;
    }
    
    // TWAI 시작
    result = twai_start();
    if (result != ESP_OK) {
        printf("CAN 시작 실패! 오류: %d\r\n", result);
        twai_driver_uninstall();
        return false;
    }
    
    initialized = true;
    
    printf("ESP32 내장 CAN 초기화 완료\r\n");
    printf("보드레이트: 500 kbps\r\n");
    printf("TX 핀: GPIO %d, RX 핀: GPIO %d\r\n", txPin, rxPin);
    
    return true;
}

bool RemoteCANCom::sendMessage(uint32_t id, const uint8_t* data, uint8_t len) {
    if (!initialized) {
        printf("CAN이 초기화되지 않음\r\n");
        return false;
    }
    
    if (len > 8) {
        printf("CAN 메시지 길이 초과 (최대 8바이트)\r\n");
        return false;
    }
    
    // TWAI 메시지 구성
    twai_message_t message;
    message.identifier = id;
    message.data_length_code = len;
    message.flags = TWAI_MSG_FLAG_NONE;  // 표준 프레임
    
    for (uint8_t i = 0; i < len; i++) {
        message.data[i] = data[i];
    }
    
    // 메시지 전송 (1초 타임아웃)
    esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(1000));
    
    if (result == ESP_OK) {
        printf("CAN 메시지 전송: ID=0x%03X, Len=%d\r\n", id, len);
        return true;
    } else {
        printf("CAN 메시지 전송 실패! 오류: %d\r\n", result);
        return false;
    }
}

bool RemoteCANCom::receiveMessage(twai_message_t& message) {
    if (!initialized) {
        return false;
    }
    
    // 메시지 수신 (타임아웃 없음)
    esp_err_t result = twai_receive(&message, 0);
    return (result == ESP_OK);
}

bool RemoteCANCom::isMessageAvailable() {
    if (!initialized) {
        return false;
    }
    
    // 상태 정보 확인
    twai_status_info_t status_info;
    esp_err_t result = twai_get_status_info(&status_info);
    
    if (result == ESP_OK) {
        return (status_info.msgs_to_rx > 0);
    }
    
    return false;
}

bool RemoteCANCom::enterSettingsMode() {
    printf("=== 설정 모드 진입 ===\r\n");
    
    settingsMode = true;
    bufferIndex = 0;
    memset(configBuffer, 0, sizeof(configBuffer));
    
    // Config Read 요청 (0x700)
    bool success = requestConfig();
    
    if (success && pLcd) {
        pLcd->clear();
        pLcd->setTextSize(2);
        pLcd->printTextCentered("설정 모드", 10, RemoteLCD::CYAN);
        pLcd->setTextSize(1);
        pLcd->printTextCentered("Config 읽는 중...", 40, RemoteLCD::YELLOW);
    }
    
    return success;
}

bool RemoteCANCom::exitSettingsMode() {
    printf("=== 설정 모드 종료 ===\r\n");
    
    settingsMode = false;
    bufferIndex = 0;
    
    if (pLcd) {
        pLcd->clear();
        pLcd->drawMainScreen();
    }
    
    return true;
}

bool RemoteCANCom::isInSettingsMode() {
    return settingsMode;
}

// 0x700: Config Read Command (retrieve=1)
bool RemoteCANCom::requestConfig() {
    printf("Config 데이터 요청 (0x700)\r\n");
    
    uint8_t buf[8];
    TX_COMMAND_BYTE cmd;
    
    memset(buf, 0, 8);
    cmd.u8 = 0;
    cmd.b.retrieve = 1;  // retrieve 비트 설정
    
    buf[0] = 0xAA;       // 시작 마커
    buf[1] = cmd.u8;     // 커맨드 바이트
    buf[7] = '@';        // 종료 마커
    
    bufferIndex = 0;
    return sendMessage(CAN_TX_GET_CONFIG, buf, 8);
}

// 0x708: Config Save Command (saving=1)
bool RemoteCANCom::saveConfig() {
    printf("Config 저장 요청 (0x708)\r\n");
    
    uint8_t buf[8];
    TX_COMMAND_BYTE cmd;
    
    memset(buf, 0, 8);
    cmd.u8 = 0;
    cmd.b.saving = 1;  // saving 비트 설정
    
    buf[0] = 0xAA;
    buf[1] = cmd.u8;
    buf[7] = '@';
    
    return sendMessage(CAN_TX_SAVE_CMD, buf, 8);
}

// 0x701: Config Write Command
bool RemoteCANCom::updateConfig(uint8_t index, const uint8_t* data, uint8_t len) {
    printf("Config 업데이트 (0x701): index=%d, len=%d\r\n", index, len);
    
    if (len > 8) {
        printf("데이터 길이 초과 (최대 8바이트)\r\n");
        return false;
    }
    
    uint8_t buf[8];
    memset(buf, 0, 8);
    
    // 첫 바이트에 인덱스, 나머지는 데이터
    buf[0] = index;
    for (uint8_t i = 0; i < len && i < 7; i++) {
        buf[i + 1] = data[i];
    }
    
    return sendMessage(CAN_TX_PUT_CMD, buf, 8);
}

// 0x5B0~0x5B7: Config Data 파싱 (8개 메시지, 총 64바이트)
bool RemoteCANCom::parseConfigData(uint32_t canId, const uint8_t* data, uint8_t len) {
    // CAN ID에서 메시지 인덱스 계산 (0x5B0~0x5B7 → 0~7)
    if (canId < CAN_RX_DATA_ID_BASE || canId > (CAN_RX_DATA_ID_BASE + 7)) {
        return false;  // 유효하지 않은 ID
    }
    
    uint8_t msgIndex = canId - CAN_RX_DATA_ID_BASE;
    uint8_t bufferOffset = msgIndex * 8;
    
    printf("Config 데이터 수신: ID=0x%03X, Index=%d\r\n", canId, msgIndex);
    
    // 버퍼에 복사 (최대 8바이트)
    uint8_t copyLen = (len > 8) ? 8 : len;
    for (uint8_t i = 0; i < copyLen; i++) {
        configBuffer[bufferOffset + i] = data[i];
    }
    
    // 수신한 메시지 카운트 증가
    if (msgIndex >= bufferIndex) {
        bufferIndex = msgIndex + 1;
    }
    
    // 8개 메시지 모두 수신 완료 확인
    if (bufferIndex >= 8) {
        printf("전체 Config 데이터 수신 완료 (64바이트)\r\n");
        
        // YbCarDoctor로 데이터 전달
        if (pDoctor) {
            pDoctor->loadConfigFromBuffer(configBuffer, 64);
        }
        
        if (pLcd) {
            pLcd->printTextCentered("Config 로드 완료!", 60, RemoteLCD::GREEN);
        }
        
        return true;
    }
    
    return false;  // 아직 모두 수신 안 됨
}

void RemoteCANCom::setHandlers(RemoteLCD* lcd, YbCarDoctor* doctor) {
    pLcd = lcd;
    pDoctor = doctor;
}

void RemoteCANCom::update() {
    if (!initialized) {
        return;
    }
    
    // 수신 메시지 처리
    while (isMessageAvailable()) {
        twai_message_t message;
        if (receiveMessage(message)) {
            processReceivedMessage(message);
            
            // 사용자 콜백 호출
            if (receiveCallback) {
                receiveCallback(message);
            }
        }
    }
}

void RemoteCANCom::setReceiveCallback(void (*callback)(const twai_message_t&)) {
    receiveCallback = callback;
}

void RemoteCANCom::processReceivedMessage(const twai_message_t& message) {
    lastMessageTime = millis();
    
    printf("CAN 메시지 수신: ID=0x%03X, Len=%d\r\n", message.identifier, message.data_length_code);
    
    // Config 데이터 메시지 (0x5B0~0x5B7)
    if (message.identifier >= CAN_RX_DATA_ID_BASE && 
        message.identifier < (CAN_RX_DATA_ID_BASE + 8)) {
        handleConfigDataMessage(message.identifier, message);
    }
    // VCU 응답 메시지 (0x5B8)
    else if (message.identifier == CAN_RX_RESPONSE_ID) {
        handleResponseMessage(message);
    }
    else {
        printf("알 수 없는 CAN ID: 0x%03X\r\n", message.identifier);
    }
}

void RemoteCANCom::handleConfigDataMessage(uint32_t canId, const twai_message_t& message) {
    // Config 데이터 파싱 (0x5B0~0x5B7)
    bool complete = parseConfigData(canId, message.data, message.data_length_code);
    
    if (complete) {
        printf("전체 Config 수신 완료\r\n");
        
        // LCD에 진행 상황 표시
        if (pLcd) {
            pLcd->setTextSize(1);
            pLcd->printTextCentered("64 바이트 수신 완료", 80, RemoteLCD::GREEN);
        }
    }
}

void RemoteCANCom::handleResponseMessage(const twai_message_t& message) {
    printf("VCU 응답 메시지 수신 (0x5B8)\r\n");
    
    // VCU 상태 정보 출력
    if (message.data_length_code >= 8) {
        printf("STATUS: 0x%02X\r\n", message.data[0]);
        printf("MOTOR_RPM: %d\r\n", message.data[1]);
        printf("MOTOR_TEMP: %d\r\n", message.data[2]);
        printf("FET_TEMP: %d\r\n", message.data[3]);
        printf("LO_Battery_Volt: %d\r\n", message.data[4]);
        printf("HI_Battery_Volt: %d\r\n", message.data[5]);
        printf("ex_pwm2: %d\r\n", message.data[6]);
        printf("ex_pwm1: %d\r\n", message.data[7]);
        
        // YbCar 또는 YbCarDoctor로 데이터 전달 가능
        if (pDoctor) {
            // VCU 상태 정보 처리
        }
    }
}
