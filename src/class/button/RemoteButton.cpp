#include "RemoteButton.h"
#include "../lcd/RemoteLCD.h"
#include "../espnow/RemoteESPNow.h"

RemoteButton::RemoteButton() {
    debounceTime = 50;          // 50ms 디바운스
    longPressTime = 1000;       // 1초 롱프레스
    doubleClickTime = 300;      // 300ms 더블클릭
    
    eventQueueHead = 0;
    eventQueueTail = 0;
    lastButtonState = 0xFFFF;   // 모든 버튼 릴리스 상태 (풀업)
    
    pLcd = nullptr;
    pEspNow = nullptr;
    
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
    // I2C 초기??
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000); // 400kHz Fast Mode
    
    delay(10);
    
    // PCA9555 초기???�인
    Wire.beginTransmission(PCA9555_ADDRESS);
    if (Wire.endTransmission() != 0) {
        Serial.println("PCA9555 초기???�패 - I2C ?�신 ?�류");
        return false;
    }
    
    // Port 0 (IOI_0 ~ IOI_7)???�력?�로 ?�정
    writePCA9555(CONFIG_PORT_0, 0xFF);
    
    // Port 1 (IOI_8 ~ IOI_11)을 입력으로 설정 (하위 4비트만)
    writePCA9555(CONFIG_PORT_1, 0x0F);
    
    Serial.println("PCA9555 버튼 초기화 완료 (12개)");
    Serial.println("버튼 매핑: IOI_0~IOI_11 (ID: 0~11)");
    
    return true;
}

void RemoteButton::scan() {
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        processButton(i);
    }
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
    
    // ?��? 가??차�? ?�았?�면 추�?
    if (nextTail != eventQueueHead) {
        eventQueue[eventQueueTail] = event;
        eventQueueTail = nextTail;
    }
}

bool RemoteButton::readButton(uint8_t buttonId) {
    if (buttonId >= BUTTON_COUNT) return false;
    
    uint16_t allButtons = readAllButtons();
    // LOW = ?�림 (?�??
    return !(allButtons & (1 << buttonId));
}

uint16_t RemoteButton::readAllButtons() {
    uint8_t port0 = readPCA9555(INPUT_PORT_0);
    uint8_t port1 = readPCA9555(INPUT_PORT_1);
    
    // 12비트�??�용 (IOI_0 ~ IOI_11)
    uint16_t buttons = ((uint16_t)port1 << 8) | port0;
    buttons &= 0x0FFF; // ?�위 12비트�?
    
    return buttons;
}

bool RemoteButton::writePCA9555(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(PCA9555_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

uint8_t RemoteButton::readPCA9555(uint8_t reg) {
    Wire.beginTransmission(PCA9555_ADDRESS);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return 0xFF; // ?�류 ??모든 버튼 릴리?�로 간주
    }
    
    Wire.requestFrom(PCA9555_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF;
}

void RemoteButton::processButton(uint8_t buttonId) {
    if (buttonId >= BUTTON_COUNT) return;
    
    ButtonState& btn = buttons[buttonId];
    bool currentState = readButton(buttonId);
    unsigned long now = millis();
    
    // ?�전 ?�태 ?�??
    btn.wasPressed = btn.isPressed;
    
    // ?�바?�싱
    if (currentState != btn.isPressed) {
        unsigned long timeSinceChange = btn.isPressed ? 
            (now - btn.pressTime) : (now - btn.releaseTime);
        
        if (timeSinceChange >= debounceTime) {
            btn.isPressed = currentState;
            
            if (btn.isPressed) {
                // 버튼 ?�림
                btn.pressTime = now;
                
                ButtonEventInfo event;
                event.buttonId = buttonId;
                event.event = BUTTON_PRESSED;
                event.duration = 0;
                addEvent(event);
                
                Serial.print("버튼 ");
                Serial.print(buttonId);
                Serial.println(" ?�림");
                
            } else {
                // 버튼 릴리??
                btn.releaseTime = now;
                unsigned long pressDuration = now - btn.pressTime;
                
                ButtonEventInfo event;
                event.buttonId = buttonId;
                event.duration = pressDuration;
                
                // 롱프?�스 ?�인
                if (pressDuration >= longPressTime) {
                    event.event = BUTTON_LONG_PRESS;
                    Serial.print("버튼 ");
                    Serial.print(buttonId);
                    Serial.println(" 롱프레스");
                } else {
                    event.event = BUTTON_RELEASED;
                    Serial.print("버튼 ");
                    Serial.print(buttonId);
                    Serial.println(" 릴리스");
                }
                
                addEvent(event);
            }
        }
    }
    
    // 롱프?�스 체크 (버튼??계속 ?�려?�는 경우)
    if (btn.isPressed && btn.wasPressed) {
        unsigned long pressDuration = now - btn.pressTime;
        
        // 롱프?�스 ?�간 ?�달 ???�번�??�벤??발생
        if (pressDuration >= longPressTime && 
            pressDuration < longPressTime + 100) {
            
            ButtonEventInfo event;
            event.buttonId = buttonId;
            event.event = BUTTON_LONG_PRESS;
            event.duration = pressDuration;
            addEvent(event);
            
            Serial.print("버튼 ");
            Serial.print(buttonId);
            Serial.println(" 롱프레스 감지");
        }
    }
}

// 핸들러 설정
void RemoteButton::setHandlers(RemoteLCD* lcd, RemoteESPNow* espNow) {
    pLcd = lcd;
    pEspNow = espNow;
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
    Serial.print("버튼 ");
    Serial.print(buttonId);
    Serial.println(" 롱프레스 - 특수 기능 실행");
    
    if (pLcd) {
        // 롱프레스 특수 기능 (예: 설정 메뉴 등)
        pLcd->setTextSize(1);
        pLcd->printTextCentered("롱프레스!", 170, RemoteLCD::MAGENTA);
        delay(500);
        pLcd->drawMainScreen();
    }
}
