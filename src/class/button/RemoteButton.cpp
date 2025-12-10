#include "RemoteButton.h"
#include "../lcd/RemoteLCD.h"
#include "../espnow/RemoteESPNow.h"
#include "../cancom/RemoteCANCom.h"

RemoteButton::RemoteButton() {
    debounceTime = 50;          // 50ms 디바운스
    longPressTime = 1000;       // 1초 롱프레스
    doubleClickTime = 300;      // 300ms 더블클릭
    
    eventQueueHead = 0;
    eventQueueTail = 0;
    lastButtonState = 0x1F;     // 5개 버튼 릴리스 상태 (풀업)
    
    pLcd = nullptr;
    pEspNow = nullptr;
    pCanCom = nullptr;
    
    tripleButtonPressStart = 0;
    settingsModeRequested = false;
    
    // 버튼 상태 초기화
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].id = i;
        buttons[i].isPressed = false;
        buttons[i].wasPressed = false;
        buttons[i].pressTime = 0;
        buttons[i].releaseTime = 0;
    }
}

bool RemoteButton::begin() {
    // GPIO 핀 초기화 (12512WS-08 5버튼)
    pinMode(PIN_BTN_SELECT, INPUT_PULLUP);  // SELECT (중앙)
    pinMode(PIN_BTN_DOWN, INPUT_PULLUP);    // DOWN
    pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);   // RIGHT
    pinMode(PIN_BTN_LEFT, INPUT_PULLUP);    // LEFT
    pinMode(PIN_BTN_UP, INPUT_PULLUP);      // UP
    
    printf("12512WS-08 버튼 초기화 완료 (5개)\r\n");
    printf("버튼 매핑:\r\n");
    printf("  SELECT(중앙): GPIO 12\r\n");
    printf("  DOWN: GPIO 13\r\n");
    printf("  RIGHT: GPIO 14\r\n");
    printf("  LEFT: GPIO 27\r\n");
    printf("  UP: GPIO 26\r\n");
    printf("설정 모드: SELECT + LEFT + RIGHT 동시 누름\r\n");
    
    return true;
}

void RemoteButton::scan() {
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        processButton(i);
    }
    
    // 설정 모드 콤보 확인
    checkSettingsModeCombo();
}

bool RemoteButton::isButtonPressed(uint8_t buttonId) {
    if (buttonId >= BUTTON_COUNT) return false;
    return buttons[buttonId].isPressed;
}

bool RemoteButton::wasButtonJustPressed(uint8_t buttonId) {
    if (buttonId >= BUTTON_COUNT) return false;
    
    ButtonState& btn = buttons[buttonId];
    bool result = btn.isPressed && !btn.wasPressed;
    
    return result;
}

bool RemoteButton::wasButtonJustReleased(uint8_t buttonId) {
    if (buttonId >= BUTTON_COUNT) return false;
    
    ButtonState& btn = buttons[buttonId];
    bool result = !btn.isPressed && btn.wasPressed;
    
    return result;
}

bool RemoteButton::hasEvent() {
    return eventQueueHead != eventQueueTail;
}

ButtonEventInfo RemoteButton::getEvent() {
    ButtonEventInfo event = {0, BUTTON_NONE, 0};
    
    if (hasEvent()) {
        event = eventQueue[eventQueueHead];
        eventQueueHead = (eventQueueHead + 1) % 20;
    }
    
    return event;
}

void RemoteButton::setDebounceTime(unsigned long ms) {
    debounceTime = ms;
}

void RemoteButton::setLongPressTime(unsigned long ms) {
    longPressTime = ms;
}

void RemoteButton::setDoubleClickTime(unsigned long ms) {
    doubleClickTime = ms;
}

void RemoteButton::addEvent(ButtonEventInfo event) {
    uint8_t nextTail = (eventQueueTail + 1) % 20;
    
    // 큐가 가득 차지 않았으면 추가
    if (nextTail != eventQueueHead) {
        eventQueue[eventQueueTail] = event;
        eventQueueTail = nextTail;
    }
}

bool RemoteButton::readButton(uint8_t buttonId) {
    if (buttonId >= BUTTON_COUNT) return false;
    
    uint8_t pin = getPinForButton(buttonId);
    // LOW = 눌림 (풀업)
    return digitalRead(pin) == LOW;
}

uint8_t RemoteButton::getPinForButton(uint8_t buttonId) {
    switch (buttonId) {
        case BTN_SELECT: return PIN_BTN_SELECT;
        case BTN_DOWN: return PIN_BTN_DOWN;
        case BTN_RIGHT: return PIN_BTN_RIGHT;
        case BTN_LEFT: return PIN_BTN_LEFT;
        case BTN_UP: return PIN_BTN_UP;
        default: return 0;
    }
}

bool RemoteButton::areButtonsPressed(uint8_t btn1, uint8_t btn2, uint8_t btn3) {
    return readButton(btn1) && readButton(btn2) && readButton(btn3);
}

void RemoteButton::checkSettingsModeCombo() {
    // SELECT + LEFT + RIGHT 동시 누름 확인
    if (areButtonsPressed(BTN_SELECT, BTN_LEFT, BTN_RIGHT)) {
        if (tripleButtonPressStart == 0) {
            tripleButtonPressStart = millis();
        } else if (millis() - tripleButtonPressStart > 1000 && !settingsModeRequested) {
            // 1초 이상 눌림 -> 설정 모드 진입
            settingsModeRequested = true;
            printf("=== 설정 모드 콤보 감지 ===\r\n");
            
            if (pCanCom) {
                pCanCom->enterSettingsMode();
            }
            
            if (pLcd) {
                pLcd->clear();
                pLcd->setTextSize(2);
                pLcd->printTextCentered("설정 모드", 10, RemoteLCD::CYAN);
                pLcd->setTextSize(1);
                pLcd->printTextCentered("CAN 연결 중...", 40, RemoteLCD::YELLOW);
            }
        }
    } else {
        tripleButtonPressStart = 0;
        if (settingsModeRequested) {
            settingsModeRequested = false;
        }
    }
}

void RemoteButton::processButton(uint8_t buttonId) {
    if (buttonId >= BUTTON_COUNT) return;
    
    ButtonState& btn = buttons[buttonId];
    bool currentState = readButton(buttonId);
    unsigned long now = millis();
    
    // 이전 상태 저장
    btn.wasPressed = btn.isPressed;
    
    // 디바운싱
    if (currentState != btn.isPressed) {
        unsigned long timeSinceChange = btn.isPressed ? 
            (now - btn.pressTime) : (now - btn.releaseTime);
        
        if (timeSinceChange >= debounceTime) {
            btn.isPressed = currentState;
            
            if (btn.isPressed) {
                // 버튼 눌림
                btn.pressTime = now;
                
                ButtonEventInfo event;
                event.buttonId = buttonId;
                event.event = BUTTON_PRESSED;
                event.duration = 0;
                addEvent(event);
                
                printf("버튼 %d 누림\r\n", buttonId);
                
            } else {
                // 버튼 릴리스
                btn.releaseTime = now;
                unsigned long pressDuration = now - btn.pressTime;
                
                ButtonEventInfo event;
                event.buttonId = buttonId;
                event.duration = pressDuration;
                
                // 롱프레스 확인
                if (pressDuration >= longPressTime) {
                    event.event = BUTTON_LONG_PRESS;
                    printf("버튼 %d 롱프레스\r\n", buttonId);
                } else {
                    event.event = BUTTON_RELEASED;
                    printf("버튼 %d 릴리스\r\n", buttonId);
                }
                
                addEvent(event);
            }
        }
    }
    
    // 롱프레스 체크 (버튼이 계속 눌려있는 경우)
    if (btn.isPressed && btn.wasPressed) {
        unsigned long pressDuration = now - btn.pressTime;
        
        // 롱프레스 시간 도달 시 한번만 이벤트 발생
        if (pressDuration >= longPressTime && 
            pressDuration < longPressTime + 100) {
            
            ButtonEventInfo event;
            event.buttonId = buttonId;
            event.event = BUTTON_LONG_PRESS;
            event.duration = pressDuration;
            addEvent(event);
            
            printf("버튼 %d 롱프레스 감지\r\n", buttonId);
        }
    }
}

// 핸들러 설정
void RemoteButton::setHandlers(RemoteLCD* lcd, RemoteESPNow* espNow, RemoteCANCom* canCom) {
    pLcd = lcd;
    pEspNow = espNow;
    pCanCom = canCom;
}

// 이벤트 자동 처리
void RemoteButton::processEvents() {
    while (hasEvent()) {
        ButtonEventInfo event = getEvent();
        
        switch (event.event) {
            case BUTTON_PRESSED:
                handleButtonPressed(event.buttonId);
                break;
                
            case BUTTON_RELEASED:
                handleButtonReleased(event.buttonId);
                break;
                
            case BUTTON_LONG_PRESS:
                handleButtonLongPress(event.buttonId);
                break;
                
            default:
                break;
        }
    }
}

// 버튼 눌림 처리
void RemoteButton::handleButtonPressed(uint8_t buttonId) {
    if (pLcd) {
        pLcd->showButtonStatus(buttonId, true);
    }
    if (pEspNow) {
        pEspNow->sendButtonPress(buttonId);
    }
}

// 버튼 릴리스 처리
void RemoteButton::handleButtonReleased(uint8_t buttonId) {
    if (pLcd) {
        pLcd->showButtonStatus(buttonId, false);
    }
    if (pEspNow) {
        pEspNow->sendButtonRelease(buttonId);
    }
}

// 버튼 롱프레스 처리
void RemoteButton::handleButtonLongPress(uint8_t buttonId) {
    printf("버튼 %d 롱프레스 - 특수 기능 실행\r\n", buttonId);
    
    if (pLcd) {
        // 롱프레스 특수 기능 (예: 설정 메뉴 등)
        pLcd->setTextSize(1);
        pLcd->printTextCentered("롱프레스!", 170, RemoteLCD::MAGENTA);
        delay(500);
        pLcd->drawMainScreen();
    }
}
