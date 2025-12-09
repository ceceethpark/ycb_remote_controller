#ifndef REMOTE_LED_H
#define REMOTE_LED_H

#include <Arduino.h>

class RemoteLED {
public:
    RemoteLED();
    
    // 초기화
    void begin();
    
    // LED 제어
    void on();
    void off();
    void toggle();
    void set(bool state);
    
    // 깜빡임
    void blink(unsigned long duration = 100);
    void blinkTimes(uint8_t times, unsigned long duration = 100);
    
    // 상태 표시
    void showSuccess();
    void showError();
    void showTransmitting();
    
    // PWM 밝기 제어 (옵션)
    void setBrightness(uint8_t brightness); // 0-255
    
    // 상태 확인
    bool isOn();
    
    // 자동 업데이트 (loop에서 호출)
    void update();
    
private:
    static const uint8_t LED_PIN = 2;  // GPIO 2 (내장 LED)
    
    bool currentState;
    bool blinkActive;
    uint8_t blinkCount;
    uint8_t blinkRemaining;
    unsigned long blinkDuration;
    unsigned long lastBlinkTime;
    
    void startBlink(uint8_t times, unsigned long duration);
};

#endif // REMOTE_LED_H
