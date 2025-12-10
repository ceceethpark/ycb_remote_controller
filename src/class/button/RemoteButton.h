#ifndef REMOTE_BUTTON_H
#define REMOTE_BUTTON_H

#include <Arduino.h>

// Forward declarations
class RemoteLCD;
class RemoteESPNow;
class RemoteCANCom;

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
    
    // 핸들러 설정
    void setHandlers(RemoteLCD* lcd, RemoteESPNow* espNow, RemoteCANCom* canCom);
    
    // 버튼 이벤트 자동 처리
    void processEvents();
    
    // 설정
    void setDebounceTime(unsigned long ms);
    void setLongPressTime(unsigned long ms);
    void setDoubleClickTime(unsigned long ms);
    
    // 버튼 조합 확인
    bool areButtonsPressed(uint8_t btn1, uint8_t btn2, uint8_t btn3);
    
    // 버튼 ID 정의 (5버튼)
    static const uint8_t BTN_SELECT = 0;  // IOI_0 (중앙)
    static const uint8_t BTN_DOWN = 1;    // IOI_1
    static const uint8_t BTN_RIGHT = 2;   // IOI_2
    static const uint8_t BTN_LEFT = 3;    // IOI_3
    static const uint8_t BTN_UP = 4;      // IOI_4
    static const uint8_t BUTTON_COUNT = 5;
    
private:
    ButtonState buttons[BUTTON_COUNT];
    ButtonEventInfo eventQueue[20];
    uint8_t eventQueueHead;
    uint8_t eventQueueTail;
    
    // 타이밍 설정
    unsigned long debounceTime;
    unsigned long longPressTime;
    unsigned long doubleClickTime;
    
    // 12512WS-08 직접 GPIO 연결
    static const uint8_t PIN_BTN_SELECT = 12;  // IOI_0
    static const uint8_t PIN_BTN_DOWN = 13;    // IOI_1
    static const uint8_t PIN_BTN_RIGHT = 14;   // IOI_2
    static const uint8_t PIN_BTN_LEFT = 27;    // IOI_3
    static const uint8_t PIN_BTN_UP = 26;      // IOI_4
    
    uint8_t lastButtonState;
    
    // 핸들러 객체 포인터
    RemoteLCD* pLcd;
    RemoteESPNow* pEspNow;
    RemoteCANCom* pCanCom;
    
    // 설정 모드 진입용
    unsigned long tripleButtonPressStart;
    bool settingsModeRequested;
    
    // 내부 함수
    void addEvent(ButtonEventInfo event);
    bool readButton(uint8_t buttonId);
    void processButton(uint8_t buttonId);
    uint8_t getPinForButton(uint8_t buttonId);
    
    // 이벤트 핸들러
    void handleButtonPressed(uint8_t buttonId);
    void handleButtonReleased(uint8_t buttonId);
    void handleButtonLongPress(uint8_t buttonId);
    void checkSettingsModeCombo();
};

#endif // REMOTE_BUTTON_H
