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
    
    // ST7789 초기화 (240x320 세로형)
    tft->init(SCREEN_WIDTH, SCREEN_HEIGHT);
    tft->setRotation(0); // 세로 모드 (Portrait)
    
    clear();
    
    printf("LCD 초기화 완료\r\n");
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
    // 버튼 표시 기능 제거 (물리 버튼만 사용, LCD에는 표시하지 않음)
    // 필요시 나중에 간단한 표시등으로 구현 가능
    (void)buttonId;
    (void)pressed;
}

void RemoteLCD::showConnectionStatus(bool connected) {
    if (!tft) return;
    
    uint16_t color = connected ? GREEN : GRAY;
    const char* status = connected ? "연결됨" : "대기중";
    
    // LED 원 그리기
    tft->fillCircle(20, 303, 5, BLACK);  // 기존 원 지우기
    tft->fillCircle(20, 303, 5, color);  // 새 상태 원
    
    // 연결 상태 영역 지우기
    tft->fillRect(30, 300, 70, 16, BLACK);
    
    // 한글로 상태 표시
    draw16String(30, 300, color, BLACK, status, 1, 0);
}

void RemoteLCD::showBatteryLevel(uint8_t percentage) {
    if (!tft) return;
    
    // 대형 배터리 진행바 (220x18)
    drawProgressBar(10, 272, 220, 18, percentage);
    
    // 퍼센트 텍스트 (진행바 중앙)
    char text[10];
    sprintf(text, "%d%%", percentage);
    
    tft->fillRect(110, 275, 30, 12, BLACK);
    tft->setCursor(110, 275);
    tft->setTextSize(1);
    tft->setTextColor(BLACK);  // 검정 배경에 검정 글자 (진행바 위에)
    tft->print(text);
}

void RemoteLCD::showRSSI(int8_t rssi) {
    if (!tft) return;
    
    // RSSI 표시 (새 위치: 110, 300)
    char text[20];
    sprintf(text, "RSSI: %d dBm", rssi);
    
    // 이전 텍스트 지우기
    tft->fillRect(110, 300, 120, 16, BLACK);
    
    // RSSI 값에 따른 색상 결정
    uint16_t color;
    if (rssi > -50) {
        color = GREEN;      // 강함
    } else if (rssi > -70) {
        color = YELLOW;     // 보통
    } else {
        color = RED;        // 약함
    }
    
    tft->setCursor(110, 300);
    tft->setTextSize(1);
    tft->setTextColor(color);
    tft->print(text);
}

void RemoteLCD::showVehicleSpeed(uint8_t speed) {
    if (!tft) return;
    
    // 속도 숫자 영역 지우기 (초대형 4배 크기)
    tft->fillRect(80, 90, 160, 35, BLACK);
    
    // 속도 표시 (초대형 72pt, 중앙 정렬)
    char text[10];
    sprintf(text, "%d", speed);
    
    tft->setTextSize(4);
    tft->setTextColor(GREEN);
    tft->setCursor(90, 90);
    tft->print(text);
}

void RemoteLCD::showVehicleDirection(uint8_t direction) {
    if (!tft) return;
    
    const char* dirText;
    uint16_t color;
    
    switch (direction) {
        case 0: dirText = "● 정지"; color = YELLOW; break;
        case 1: dirText = "▲ 전진"; color = GREEN; break;
        case 2: dirText = "▼ 후진"; color = RED; break;
        default: dirText = "?? ??"; color = GRAY; break;
    }
    
    // 방향 표시 영역 지우기
    tft->fillRect(85, 130, 150, 20, BLACK);
    
    // 방향 아이콘 + 한글 (2배 확대)
    draw16String(90, 130, color, BLACK, dirText, 2, 0);
}

void RemoteLCD::showVehicleBattery(uint8_t percentage) {
    // showBatteryLevel()로 대체됨
    showBatteryLevel(percentage);
}

void RemoteLCD::showMotorTemp(int16_t temp) {
    if (!tft) return;
    
    char text[10];
    sprintf(text, "%d°C", temp);
    
    // 모터 온도 영역 지우기 (값만)
    tft->fillRect(50, 220, 70, 16, BLACK);
    
    // 색상 결정 (정상/경고/위험)
    uint16_t color = temp > 80 ? RED : (temp > 70 ? YELLOW : GREEN);
    draw16String(50, 220, color, BLACK, text, 1, 0);
}

void RemoteLCD::showMotorCurrent(uint16_t current) {
    if (!tft) return;
    
    char text[15];
    float currentFloat = current / 100.0;
    sprintf(text, "%.1fA", currentFloat);
    
    // 전류 영역 지우기 (값만)
    tft->fillRect(50, 240, 80, 16, BLACK);
    
    // CYAN 색상으로 표시
    draw16String(50, 240, CYAN, BLACK, text, 1, 0);
}

void RemoteLCD::showFetTemp(int16_t temp) {
    if (!tft) return;
    
    char text[10];
    sprintf(text, "%d°C", temp);
    
    // FET 온도 영역 지우기 (값만)
    tft->fillRect(165, 220, 70, 16, BLACK);
    
    // 색상 결정
    uint16_t color = temp > 80 ? RED : (temp > 70 ? YELLOW : GREEN);
    draw16String(165, 220, color, BLACK, text, 1, 0);
}

void RemoteLCD::drawMainScreen() {
    if (!tft) return;
    
    clear();
    
    // ========== 헤더 (0-60) ==========
    // 제목 (한글 16x16, 3배 확대 = 48x48)
    int titleWidth = draw16Length("YCB AI 전동차", 1.5);
    int titleX = (SCREEN_WIDTH - titleWidth) / 2;
    draw16String(titleX, 10, CYAN, BLACK, "YCB AI 전동차", 1.5, 0);
    
    tft->drawFastHLine(0, 60, SCREEN_WIDTH, GRAY);
    
    // ========== 속도 영역 (70-150) ==========
    // 라벨 "속도 (km/h)"
    int speedLabelWidth = draw16Length("속도 (km/h)", 1);
    int speedLabelX = (SCREEN_WIDTH - speedLabelWidth) / 2;
    draw16String(speedLabelX, 70, WHITE, BLACK, "속도 (km/h)", 1, 0);
    
    // 속도 숫자 (초대형 4배) - 초기값 0
    tft->setTextSize(4);
    tft->setTextColor(GREEN);
    tft->setCursor(90, 90);
    tft->print("0");
    
    // 방향 표시 (초기: 정지)
    draw16String(90, 130, YELLOW, BLACK, "● 정지", 2, 0);
    
    
    // ========== 온도/전류 영역 (220-255) ==========
    // 첫 번째 줄: 모터, FET
    draw16String(10, 220, GRAY, BLACK, "모터", 1, 0);
    draw16String(50, 220, WHITE, BLACK, "--°C", 1, 0);
    draw16String(135, 220, GRAY, BLACK, "FET", 1, 0);
    draw16String(165, 220, WHITE, BLACK, "--°C", 1, 0);
    
    // 두 번째 줄: 전류
    draw16String(10, 240, GRAY, BLACK, "전류", 1, 0);
    draw16String(50, 240, CYAN, BLACK, "--A", 1, 0);
    
    // 배터리 레이블
    draw16String(10, 260, WHITE, BLACK, "배터리", 1, 0);
    
    // 배터리 진행바 (대형 220x20)
    drawProgressBar(10, 272, 220, 18, 100);
    
    
    // ========== 통신 상태 (295-) ==========
    // 연결 상태 (LED + 텍스트)
    tft->fillCircle(20, 303, 5, GRAY);  // 초기 상태: 회색
    draw16String(30, 300, GRAY, BLACK, "대기중", 1, 0);
    
    // RSSI
    tft->setCursor(110, 300);
    tft->setTextSize(1);
    tft->setTextColor(GRAY);
    tft->print("RSSI: --");
    
    // 설정 힌트
    draw16String(20, 315, GRAY, BLACK, "SELECT 3초 길게 누르면 설정", 0.6, 0);
    draw16String(10, 280, GRAY, BLACK, "ESP-NOW + CAN(500k)", 1, 0);
}

// drawButton 함수 제거됨 - 버튼 표시를 LCD에서 하지 않음

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

// =============================================================================
// 한글 폰트 지원 (16x16 조합형)
// =============================================================================

#include "font/english.h"
#include "font/kssm_font.h"

void RemoteLCD::setPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (tft) {
        tft->drawPixel(x, y, color);
    }
}

void RemoteLCD::fillRect16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (tft) {
        tft->fillRect(x, y, w, h, color);
    }
}

// 8x16 영문 렌더링
void RemoteLCD::draw16English(uint8_t ch) {
    if (!tft) return;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            for (int xx = 0; xx < _times; xx++) {
                for (int k = 0; k < 8; k++) {
                    uint8_t data = (english[ch][i * 8 + k]) & (1 << j);
                    if (data) {
                        for (int t = 0; t < _times; t++) {
                            setPixel(_xchar + j * _times + xx, _ychar + (i * 8 + k) * _times + t, fg_color);
                        }
                    }
                    else if (!_nobg) {
                        for (int t = 0; t < _times; t++) {
                            setPixel(_xchar + j * _times + xx, _ychar + (i * 8 + k) * _times + t, bg_color);
                        }
                    }
                }
            }
        }
    }
    
    _xchar += 8 * _times;
    if (_xchar > (SCREEN_WIDTH - (8 * _times))) {
        _xchar = 0;
        _ychar += 16 * _times;
    }
}

// 16x16 한글 조합형 렌더링
void RemoteLCD::draw16Korean(uint16_t hangeul) {
    if (!tft) return;
    
    uint8_t cho1[22] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 };
    uint8_t cho2[22] = { 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 };
    uint8_t jong[22] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 };

    uint8_t first = 0, mid = 0, last = 0;
    uint8_t firstType = 0, midType = 0, lastType = 0;
    uint8_t Korean_buffer[32] = {0,};

    // 초성(20) : 없음ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
    // 중성(22) : 없음ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ
    // 종성(28) : 없음ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ

    hangeul -= 0xAC00;    // 0xAC00='가' ~  0xD7A3='힣' 11,172글자
    last = hangeul % 28;  // 종성
    hangeul /= 28;
    first = hangeul / 21 + 1;  // 초성
    mid   = hangeul % 21 + 1;  // 중성

    if (last == 0) {  // 받침 없는 경우
        firstType = cho1[mid];
        if (first == 1 || first == 24) midType = 0;
        else midType = 1;
    }
    else {  // 받침 있는 경우
        firstType = cho2[mid];
        if (first == 1 || first == 24) midType = 2;
        else midType = 3;
        lastType = jong[mid];
    }

    // 초성(0~159)
    uint16_t pF_temp = firstType * 20 + first;
    for (int i = 0; i < 32; i++) Korean_buffer[i] = (K_font[pF_temp][i]);

    // 중성(160~247)
    pF_temp = 160 + midType * 22 + mid;
    for (int i = 0; i < 32; i++) Korean_buffer[i] |= (K_font[pF_temp][i]);

    // 종성(247~359)
    if (last) {
        pF_temp = 248 + lastType * 28 + last;
        for (int i = 0; i < 32; i++) Korean_buffer[i] |= (K_font[pF_temp][i]);
    }

    // TFT-LCD 출력
    for (int i = 0; i < 16; i++) {
        for (int xx = 0; xx < _times; xx++) {
            uint16_t data = (Korean_buffer[i] << 8) | Korean_buffer[i + 16];
            for (int j = 0; j < 16; j++) {
                if (data & 0x8000) {
                    for (int t = 0; t < _times; t++) {
                        setPixel(_xchar + j * _times, _ychar + i * _times + xx, fg_color);
                    }
                }
                else if (!_nobg) {
                    for (int t = 0; t < _times; t++) {
                        setPixel(_xchar + j * _times, _ychar + i * _times + xx, bg_color);
                    }
                }
                data <<= 1;
            }
        }
    }

    _xchar += 16 * _times;
    if (_xchar > (SCREEN_WIDTH - 16 * _times)) {
        _xchar = 0;
        _ychar += 16 * _times;
    }
}

// UTF-8 문자열 렌더링 (영문 + 한글)
int RemoteLCD::draw16String(int x, int y, uint16_t fgColor, uint16_t bgColor, const char* str, uint8_t timesX, uint8_t nobg) {
    int length = 0;
    _xchar = x;
    _ychar = y;
    fg_color = fgColor;
    bg_color = bgColor;
    _nobg = nobg;
    _times = timesX;

    while (*str) {
        uint8_t ch1 = *str++;
        if (ch1 < 0x80) {  // ASCII
            draw16English(ch1);
            length += (8 * _times);
        }
        else {  // UTF-8 한글 (3바이트)
            uint8_t ch2 = *str++;
            uint8_t ch3 = *str++;
            uint16_t hangeul = (ch1 & 0x0F) << 12 | (ch2 & 0x3F) << 6 | (ch3 & 0x3F);
            draw16Korean(hangeul);
            length += (16 * _times);
        }
    }
    return length;
}

// 문자열 픽셀 길이 계산
int RemoteLCD::draw16Length(const char* str, uint8_t timesX) {
    int length = 0;
    while (*str) {
        uint8_t ch1 = *str++;
        if (ch1 < 0x80) {  // ASCII
            length += (8 * timesX);
        }
        else {  // UTF-8 한글 (3바이트)
            str++;  // ch2
            str++;  // ch3
            length += (16 * timesX);
        }
    }
    return length;
}
