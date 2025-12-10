#ifndef REMOTE_LCD_H
#define REMOTE_LCD_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

class RemoteLCD {
public:
    RemoteLCD();
    
    // 초기화
    bool begin();
    
    // 화면 제어
    void clear();
    void setBrightness(uint8_t brightness);
    void displayOn();
    void displayOff();
    
    // 텍스트 출력
    void printText(const char* text, uint16_t x, uint16_t y, uint16_t color = 0xFFFF);
    void printTextCentered(const char* text, uint16_t y, uint16_t color = 0xFFFF);
    void setTextSize(uint8_t size);
    
    // 한글 지원 (16x16 조합형 폰트)
    int draw16String(int x, int y, uint16_t fgColor, uint16_t bgColor, const char* str, uint8_t timesX = 1, uint8_t nobg = 0);
    int draw16Length(const char* str, uint8_t timesX = 1);
    
    // 상태 표시
    void showButtonStatus(uint8_t buttonId, bool pressed);
    void showConnectionStatus(bool connected);
    void showBatteryLevel(uint8_t percentage);
    void showRSSI(int8_t rssi);
    
    // 차량 상태 표시
    void showVehicleSpeed(uint8_t speed);
    void showVehicleDirection(uint8_t direction);
    void showVehicleBattery(uint8_t percentage);
    void showMotorTemp(int16_t temp);
    void showMotorCurrent(uint16_t current);
    void showFetTemp(int16_t temp);
    
    // UI 그리기
    void drawMainScreen();
    void drawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                    const char* label, bool pressed);
    void drawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                         uint8_t percentage);
    
    // 색상 정의
    enum Color {
        BLACK = 0x0000,
        WHITE = 0xFFFF,
        RED = 0xF800,
        GREEN = 0x07E0,
        BLUE = 0x001F,
        YELLOW = 0xFFE0,
        CYAN = 0x07FF,
        MAGENTA = 0xF81F,
        GRAY = 0x8410
    };
    
private:
    Adafruit_ST7789* tft;
    uint8_t currentTextSize;
    
    // 한글 폰트 렌더링 변수
    int _xchar;
    int _ychar;
    uint8_t _nobg;
    uint8_t _times;
    uint16_t fg_color;
    uint16_t bg_color;
    
    // 한글 폰트 내부 함수
    void draw16English(uint8_t ch);
    void draw16Korean(uint16_t hangeul);
    void setPixel(uint16_t x, uint16_t y, uint16_t color);
    void fillRect16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    
    // 핀 정의
    static const uint8_t TFT_CS   = 5;
    static const uint8_t TFT_DC   = 4;
    static const uint8_t TFT_RST  = 15;
    static const uint8_t TFT_MOSI = 23;
    static const uint8_t TFT_SCLK = 18;
    
    // 화면 크기 (세로형)
    static const uint16_t SCREEN_WIDTH  = 240;
    static const uint16_t SCREEN_HEIGHT = 320;
};

#endif // REMOTE_LCD_H
