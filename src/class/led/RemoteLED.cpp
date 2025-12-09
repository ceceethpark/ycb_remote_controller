#include "RemoteLED.h"

RemoteLED::RemoteLED() {
    currentState = false;
    blinkActive = false;
    blinkCount = 0;
    blinkRemaining = 0;
    blinkDuration = 100;
    lastBlinkTime = 0;
}

void RemoteLED::begin() {
    pinMode(LED_PIN, OUTPUT);
    off();
    Serial.println("LED 초기화 완료");
}

void RemoteLED::on() {
    currentState = true;
    digitalWrite(LED_PIN, HIGH);
}

void RemoteLED::off() {
    currentState = false;
    digitalWrite(LED_PIN, LOW);
}

void RemoteLED::toggle() {
    if (currentState) {
        off();
    } else {
        on();
    }
}

void RemoteLED::set(bool state) {
    if (state) {
        on();
    } else {
        off();
    }
}

void RemoteLED::blink(unsigned long duration) {
    on();
    delay(duration);
    off();
}

void RemoteLED::blinkTimes(uint8_t times, unsigned long duration) {
    startBlink(times, duration);
}

void RemoteLED::showSuccess() {
    // 짧게 2번 깜빡임
    blinkTimes(2, 50);
}

void RemoteLED::showError() {
    // 길게 3번 깜빡임
    blinkTimes(3, 200);
}

void RemoteLED::showTransmitting() {
    // 매우 짧게 1번 깜빡임
    blink(50);
}

void RemoteLED::setBrightness(uint8_t brightness) {
    // PWM을 사용한 밝기 조절 (LED_PIN이 PWM 지원하는 경우)
    analogWrite(LED_PIN, brightness);
}

bool RemoteLED::isOn() {
    return currentState;
}

void RemoteLED::update() {
    if (!blinkActive) return;
    
    unsigned long now = millis();
    
    if (now - lastBlinkTime >= blinkDuration) {
        if (currentState) {
            // LED가 켜져있으면 끄기
            off();
            blinkRemaining--;
            
            if (blinkRemaining == 0) {
                // 깜빡임 완료
                blinkActive = false;
            }
        } else {
            // LED가 꺼져있으면 켜기
            on();
        }
        
        lastBlinkTime = now;
    }
}

void RemoteLED::startBlink(uint8_t times, unsigned long duration) {
    blinkActive = true;
    blinkCount = times;
    blinkRemaining = times;
    blinkDuration = duration;
    lastBlinkTime = millis();
    off(); // 시작은 꺼진 상태에서
}
