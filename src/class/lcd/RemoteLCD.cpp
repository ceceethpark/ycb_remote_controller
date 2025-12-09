#include "RemoteLCD.h"

RemoteLCD::RemoteLCD() {
    currentTextSize = 1;
    tft = nullptr;
}

bool RemoteLCD::begin() {
    // SPI 초기화
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    
    // TFT 객체 생성
    tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
    
    if (!tft) {
        return false;
    }
    
    // ST7789 초기화 (320x240)
    tft->init(SCREEN_WIDTH, SCREEN_HEIGHT);
    tft->setRotation(1); // 가로 모드
    
    clear();
    
    Serial.println("LCD 초기화 완료");
    return true;
}

void RemoteLCD::clear() {
    if (tft) {
        tft->fillScreen(BLACK);
    }
}

void RemoteLCD::setBrightness(uint8_t brightness) {
    // ST7789는 백라이트 제어를 위한 별도 핀 필요
    // 필요시 PWM으로 구현
}

void RemoteLCD::displayOn() {
    if (tft) {
        tft->enableDisplay(true);
    }
}

void RemoteLCD::displayOff() {
    if (tft) {
        tft->enableDisplay(false);
    }
}

void RemoteLCD::printText(const char* text, uint16_t x, uint16_t y, uint16_t color) {
    if (tft) {
        tft->setCursor(x, y);
        tft->setTextColor(color);
        tft->setTextSize(currentTextSize);
        tft->print(text);
    }
}

void RemoteLCD::printTextCentered(const char* text, uint16_t y, uint16_t color) {
    if (tft) {
        int16_t x1, y1;
        uint16_t w, h;
        tft->setTextSize(currentTextSize);
        tft->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        
        uint16_t x = (SCREEN_WIDTH - w) / 2;
        printText(text, x, y, color);
    }
}

void RemoteLCD::setTextSize(uint8_t size) {
    currentTextSize = size;
}

void RemoteLCD::showButtonStatus(uint8_t buttonId, bool pressed) {
    if (!tft) return;
    
    // 12개 버튼을 3행 4열로 배치
    uint8_t col = buttonId % 4;
    uint8_t row = buttonId / 4;
    
    uint16_t x = 10 + col * 77;
    uint16_t y = 80 + row * 45;
    
    char label[10];
    sprintf(label, "K%d", buttonId);
    
    drawButton(x, y, 70, 35, label, pressed);
}

void RemoteLCD::showConnectionStatus(bool connected) {
    if (!tft) return;
    
    uint16_t color = connected ? GREEN : RED;
    const char* status = connected ? "연결됨" : "연결 안됨";
    
    tft->fillRect(200, 10, 110, 30, BLACK);
    printText(status, 210, 15, color);
}

void RemoteLCD::showBatteryLevel(uint8_t percentage) {
    if (!tft) return;
    
    drawProgressBar(10, 10, 100, 20, percentage);
    
    char text[10];
    sprintf(text, "%d%%", percentage);
    printText(text, 120, 12, WHITE);
}

void RemoteLCD::showRSSI(int8_t rssi) {
    if (!tft) return;
    
    // RSSI 표시 위치 (배터리 아래)
    char text[20];
    sprintf(text, "RSSI:%ddBm", rssi);
    
    // 이전 텍스트 지우기 (검은색 박스)
    tft->fillRect(200, 10, 110, 20, BLACK);
    
    // RSSI 값에 따른 색상 결정
    uint16_t color;
    if (rssi > -50) {
        color = GREEN;      // 강함
    } else if (rssi > -70) {
        color = YELLOW;     // 보통
    } else {
        color = RED;        // 약함
    }
    
    printText(text, 200, 12, color);
}

void RemoteLCD::showVehicleSpeed(uint8_t speed) {
    if (!tft) return;
    
    // 속도 표시 위치 (중앙 상단)
    char text[20];
    sprintf(text, "속도:%d", speed);
    
    // 이전 텍스트 지우기
    tft->fillRect(80, 50, 160, 30, BLACK);
    
    // 큰 글씨로 표시
    tft->setTextSize(3);
    tft->setTextColor(CYAN);
    int16_t x1, y1;
    uint16_t tw, th;
    tft->getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);
    tft->setCursor(160 - tw/2, 55);
    tft->print(text);
    
    tft->setTextSize(2);
    tft->getTextBounds("km/h", 0, 0, &x1, &y1, &tw, &th);
    tft->setCursor(160 - tw/2, 80);
    tft->print("km/h");
}

void RemoteLCD::showVehicleDirection(uint8_t direction) {
    if (!tft) return;
    
    const char* dirText;
    uint16_t color;
    
    switch (direction) {
        case 0: dirText = "정지"; color = YELLOW; break;
        case 1: dirText = "전진"; color = GREEN; break;
        case 2: dirText = "후진"; color = RED; break;
        default: dirText = "??"; color = GRAY; break;
    }
    
    // 방향 표시 위치 (속도 아래)
    tft->fillRect(120, 105, 80, 20, BLACK);
    
    tft->setTextSize(2);
    tft->setTextColor(color);
    int16_t x1, y1;
    uint16_t tw, th;
    tft->getTextBounds(dirText, 0, 0, &x1, &y1, &tw, &th);
    tft->setCursor(160 - tw/2, 105);
    tft->print(dirText);
}

void RemoteLCD::showVehicleBattery(uint8_t percentage) {
    if (!tft) return;
    
    // 차량 배터리 표시 (왼쪽 중앙)
    char text[20];
    sprintf(text, "차량:%d%%", percentage);
    
    tft->fillRect(10, 130, 100, 20, BLACK);
    
    uint16_t color;
    if (percentage > 50) color = GREEN;
    else if (percentage > 20) color = YELLOW;
    else color = RED;
    
    drawProgressBar(10, 135, 90, 15, percentage);
    printText(text, 105, 137, color);
}

void RemoteLCD::showMotorTemp(int16_t temp) {
    if (!tft) return;
    
    char text[20];
    sprintf(text, "M:%dC", temp);
    
    tft->fillRect(10, 160, 100, 20, BLACK);
    
    uint16_t color = temp > 80 ? RED : (temp > 60 ? YELLOW : GREEN);
    printText(text, 10, 162, color);
}

void RemoteLCD::showMotorCurrent(uint16_t current) {
    if (!tft) return;
    
    char text[20];
    sprintf(text, "I:%dmA", current);
    
    tft->fillRect(120, 160, 100, 20, BLACK);
    printText(text, 120, 162, CYAN);
}

void RemoteLCD::showFetTemp(int16_t temp) {
    if (!tft) return;
    
    char text[20];
    sprintf(text, "F:%dC", temp);
    
    tft->fillRect(230, 160, 80, 20, BLACK);
    
    uint16_t color = temp > 80 ? RED : (temp > 60 ? YELLOW : GREEN);
    printText(text, 230, 162, color);
}

void RemoteLCD::drawMainScreen() {
    if (!tft) return;
    
    clear();
    
    // 제목
    setTextSize(2);
    printTextCentered("ESP32 리모컨", 15, CYAN);
    
    // 12개 버튼 표시 (3행 4열)
    setTextSize(1);
    for (uint8_t i = 0; i < 12; i++) {
        showButtonStatus(i, false);
    }
    
    // 상태바
    showConnectionStatus(false);
    showBatteryLevel(100);
    
    // 안내 메시지
    setTextSize(1);
    printTextCentered("PCA9555 12키 리모컨", 220, GRAY);
}

void RemoteLCD::drawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                           const char* label, bool pressed) {
    if (!tft) return;
    
    uint16_t bgColor = pressed ? GREEN : BLUE;
    uint16_t borderColor = pressed ? WHITE : CYAN;
    
    // 배경
    tft->fillRect(x, y, w, h, bgColor);
    
    // 테두리
    tft->drawRect(x, y, w, h, borderColor);
    tft->drawRect(x+1, y+1, w-2, h-2, borderColor);
    
    // 레이블
    int16_t x1, y1;
    uint16_t tw, th;
    tft->setTextSize(1);
    tft->getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
    
    uint16_t tx = x + (w - tw) / 2;
    uint16_t ty = y + (h - th) / 2;
    
    tft->setCursor(tx, ty);
    tft->setTextColor(WHITE);
    tft->print(label);
}

void RemoteLCD::drawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                                uint8_t percentage) {
    if (!tft) return;
    
    // 테두리
    tft->drawRect(x, y, w, h, WHITE);
    
    // 배경
    tft->fillRect(x+2, y+2, w-4, h-4, BLACK);
    
    // 진행 바
    uint16_t barWidth = ((w - 4) * percentage) / 100;
    uint16_t barColor = percentage > 20 ? GREEN : RED;
    
    tft->fillRect(x+2, y+2, barWidth, h-4, barColor);
}
