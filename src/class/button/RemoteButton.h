#ifndef REMOTE_BUTTON_H
#define REMOTE_BUTTON_H

#include <Arduino.h>
#include <Wire.h>

// Forward declarations
class RemoteLCD;
class RemoteESPNow;

// 버튼 상태 구조체
struct ButtonState {
    uint8_t id;
    bool isPressed;
    bool wasPressed;
    unsigned long pressTime;
    unsigned long releaseTime;
};

// 버튼 이벤트 타입
enum ButtonEvent {
    BUTTON_NONE = 0,
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    BUTTON_LONG_PRESS,
    BUTTON_DOUBLE_CLICK
};

// 버튼 이벤트 정보
struct ButtonEventInfo {
    uint8_t buttonId;
    ButtonEvent event;
    unsigned long duration;
};

class RemoteButton {
public:
    RemoteButton();
    
    // 초기화
    bool begin();
    
    // 버튼 스캔
    void scan();
    
    // 버튼 상태 확인
    bool isButtonPressed(uint8_t buttonId);
    bool wasButtonJustPressed(uint8_t buttonId);
    bool wasButtonJustReleased(uint8_t buttonId);
    
    // 이벤트 처리
    bool hasEvent();
    ButtonEventInfo getEvent();
    
    // 핸들러 설정 (LCD와 ESP-NOW 객체 연결)
    void setHandlers(RemoteLCD* lcd, RemoteESPNow* espNow);
    
    // 버튼 이벤트 자동 처리 (핸들러가 설정된 경우)
    void processEvents();
    
    // 설정
    void setDebounceTime(unsigned long ms);
    void setLongPressTime(unsigned long ms);
    void setDoubleClickTime(unsigned long ms);
    
    // 버튼 ID 정의 (IOI_0 ~ IOI_11, 12개)
    static const uint8_t BUTTON_1 = 0;   // IOI_0
    static const uint8_t BUTTON_2 = 1;   // IOI_1
    static const uint8_t BUTTON_3 = 2;   // IOI_2
    static const uint8_t BUTTON_4 = 3;   // IOI_3
    static const uint8_t BUTTON_5 = 4;   // IOI_4
    static const uint8_t BUTTON_6 = 5;   // IOI_5
    static const uint8_t BUTTON_7 = 6;   // IOI_6
    static const uint8_t BUTTON_8 = 7;   // IOI_7
    static const uint8_t BUTTON_9 = 8;   // IOI_8
    static const uint8_t BUTTON_10 = 9;  // IOI_9
    static const uint8_t BUTTON_11 = 10; // IOI_10
    static const uint8_t BUTTON_12 = 11; // IOI_11
    static const uint8_t BUTTON_COUNT = 12;
    
private:
    ButtonState buttons[BUTTON_COUNT];
    ButtonEventInfo eventQueue[20];
    uint8_t eventQueueHead;
    uint8_t eventQueueTail;
    
    // 타이밍 설정
    unsigned long debounceTime;
    unsigned long longPressTime;
    unsigned long doubleClickTime;
    
    // PCA9555 I2C 설정
    static const uint8_t PCA9555_ADDRESS = 0x20;  // 7-bit address
    static const uint8_t INPUT_PORT_0 = 0x00;     // Input port 0 register
    static const uint8_t INPUT_PORT_1 = 0x01;     // Input port 1 register
    static const uint8_t OUTPUT_PORT_0 = 0x02;    // Output port 0 register
    static const uint8_t OUTPUT_PORT_1 = 0x03;    // Output port 1 register
    static const uint8_t CONFIG_PORT_0 = 0x06;    // Configuration port 0 (1=input)
    static const uint8_t CONFIG_PORT_1 = 0x07;    // Configuration port 1 (1=input)
    
    // I2C 핀
    static const uint8_t I2C_SDA = 21;
    static const uint8_t I2C_SCL = 22;
    
    uint16_t lastButtonState;
    
    // LCD와 ESP-NOW 객체 포인터
    RemoteLCD* pLcd;
    RemoteESPNow* pEspNow;
    
    // 내부 함수
    void addEvent(ButtonEventInfo event);
    bool readButton(uint8_t buttonId);
    void processButton(uint8_t buttonId);
    
    // 이벤트 핸들러
    void handleButtonPressed(uint8_t buttonId);
    void handleButtonReleased(uint8_t buttonId);
    void handleButtonLongPress(uint8_t buttonId);
    uint16_t readAllButtons();
    bool writePCA9555(uint8_t reg, uint8_t value);
    uint8_t readPCA9555(uint8_t reg);
};

#endif // REMOTE_BUTTON_H
