# ?�래??구조 ?�명

## ?�� ?�로?�트 구조

```
esp32_core_remocon/
?��??� include/
??  ?��??� class/
??      ?��??� lcd/
??      ??  ?��??� RemoteLCD.h          # LCD ?�래???�더
??      ?��??� button/
??          ?��??� RemoteButton.h     # ?�보???�래???�더
?��??� src/
??  ?��??� class/
??  ??  ?��??� lcd/
??  ??  ??  ?��??� RemoteLCD.cpp        # LCD ?�래??구현
??  ??  ?��??� button/
??  ??      ?��??� RemoteButton.cpp   # ?�보???�래??구현
??  ?��??� main.cpp                     # 메인 ?�로그램
?��??� examples/
??  ?��??� receiver.cpp                 # ?�신�??�제
?��??� docs/
??  ?��??� pinout.md                    # ?� 배치
??  ?��??� quick-start.md               # 빠른 ?�작
??  ?��??� class-structure.md           # ?�래??구조 (???�일)
?��??� platformio.ini                   # PlatformIO ?�정
?��??� README.md                        # ?�로?�트 문서
```

## ?�� RemoteLCD ?�래??

### ?�치
- ?�더: `include/class/lcd/RemoteLCD.h`
- 구현: `src/class/lcd/RemoteLCD.cpp`

### 주요 기능
```cpp
class RemoteLCD {
public:
    // 초기??
    bool begin();
    
    // ?�면 ?�어
    void clear();
    void displayOn();
    void displayOff();
    
    // ?�스??출력
    void printText(const char* text, uint16_t x, uint16_t y, uint16_t color);
    void printTextCentered(const char* text, uint16_t y, uint16_t color);
    void setTextSize(uint8_t size);
    
    // ?�태 ?�시
    void showButtonStatus(uint8_t buttonId, bool pressed);
    void showConnectionStatus(bool connected);
    void showBatteryLevel(uint8_t percentage);
    
    // UI 그리�?
    void drawMainScreen();
    void drawButton(...);
    void drawProgressBar(...);
};
```

### ?�용 ?�제
```cpp
RemoteLCD lcd;

void setup() {
    lcd.begin();
    lcd.drawMainScreen();
    lcd.showConnectionStatus(true);
    lcd.showBatteryLevel(100);
}

void loop() {
    lcd.showButtonStatus(1, true);  // 버튼 1 ?�림 ?�시
}
```

### LCD ?�펙
- **모델**: SZH-EK096 (ST7789 컨트롤러)
- **?�신**: SPI (4 I/O)
- **?�상??*: 320 x 240
- **?�원**: DC 3.3V
- **?� ?�결**:
  - CS: GPIO 5
  - DC: GPIO 4
  - RST: GPIO 15
  - MOSI: GPIO 23
  - SCLK: GPIO 18

## ?�� RemoteButton ?�래??

### ?�치
- ?�더: `include/class/button/RemoteButton.h`
- 구현: `src/class/button/RemoteButton.cpp`

### 주요 기능
```cpp
class RemoteButton {
public:
    // 초기??
    void begin();
    
    // 버튼 ?�캔
    void scan();
    
    // 버튼 ?�태 ?�인
    bool isButtonPressed(uint8_t buttonId);
    bool wasButtonJustPressed(uint8_t buttonId);
    bool wasButtonJustReleased(uint8_t buttonId);
    
    // ?�벤??처리
    bool hasEvent();
    ButtonEventInfo getEvent();
    
    // ?�정
    void setDebounceTime(unsigned long ms);
    void setLongPressTime(unsigned long ms);
    void setDoubleClickTime(unsigned long ms);
};
```

### ?�벤???�??
```cpp
enum ButtonEvent {
    BUTTON_NONE = 0,
    BUTTON_PRESSED,       // 버튼 ?�림
    BUTTON_RELEASED,      // 버튼 릴리??
    BUTTON_LONG_PRESS,    // 롱프?�스 (1�?
    BUTTON_DOUBLE_CLICK   // ?�블?�릭 (300ms ?�내)
};
```

### ?�용 ?�제
```cpp
RemoteButton keyboard;

void setup() {
    keyboard.begin();
    keyboard.setLongPressTime(1000);  // 1�?롱프?�스
}

void loop() {
    keyboard.scan();
    
    while (keyboard.hasEvent()) {
        ButtonEventInfo event = keyboard.getEvent();
        
        switch (event.event) {
            case BUTTON_PRESSED:
                Serial.println("버튼 ?�림!");
                break;
            case BUTTON_LONG_PRESS:
                Serial.println("롱프?�스!");
                break;
        }
    }
}
```

### 버튼 ?� 매핑 (PCA9555 I2C)
- I2C SDA: GPIO 21
- I2C SCL: GPIO 22
- PCA9555 주소: 0x20
- 버튼 0~11: IOI_0 ~ IOI_11 (12�?

## ?�� main.cpp ?�합

### ?�체 ?�름
```cpp
#include "class/lcd/RemoteLCD.h"
#include "class/button/RemoteButton.h"

RemoteLCD lcd;
RemoteButton keyboard;

void setup() {
    // LCD 초기??
    lcd.begin();
    lcd.drawMainScreen();
    
    // ?�보??초기??
    keyboard.begin();
    
    // ESP-NOW 초기??
    setupESPNow();
}

void loop() {
    // ?�보???�캔
    keyboard.scan();
    
    // ?�벤??처리
    while (keyboard.hasEvent()) {
        ButtonEventInfo event = keyboard.getEvent();
        handleButtonEvent(event);
    }
}

void handleButtonEvent(ButtonEventInfo event) {
    switch (event.event) {
        case BUTTON_PRESSED:
            lcd.showButtonStatus(event.buttonId, true);
            sendButtonPress(event.buttonId);
            break;
            
        case BUTTON_RELEASED:
            lcd.showButtonStatus(event.buttonId, false);
            break;
    }
}
```

## ?�� ?�계 철학

### 1. 캡슐??
- LCD 관??모든 기능?� `RemoteLCD` ?�래?�에
- ?�보??관??모든 기능?� `RemoteButton` ?�래?�에
- main.cpp???�합 로직�??�당

### 2. 직�??�인 ?�이�?
- `RemoteLCD` - 리모컨의 LCD
- `RemoteButton` - 리모컨의 ?�보??
- 메서???�름??명확?�게 (`showButtonStatus`, `drawMainScreen`)

### 3. ?�장??
- ?�로??기능 추�?가 ?��?
- ?�래???�위�??�스??가??
- ?�른 ?�로?�트???�사??가??

### 4. ?�더 구조
```
class/
?��??� lcd/           # LCD 관??모든 ?�일
??  ?��??� RemoteLCD.h
??  ?��??� RemoteLCD.cpp
?��??� button/      # ?�보??관??모든 ?�일
    ?��??� RemoteButton.h
    ?��??� RemoteButton.cpp
```

## ?? ?�장 가?�한 기능

### LCD ?�래???�장
```cpp
// 메뉴 ?�스??추�?
void RemoteLCD::showMenu(MenuItem* items, uint8_t count);

// 그래??그리�?
void RemoteLCD::drawGraph(int* data, uint8_t count);

// ?�이�??�시
void RemoteLCD::drawIcon(Icon icon, uint16_t x, uint16_t y);
```

### ?�보???�래???�장
```cpp
// 버튼 조합 지??
bool RemoteButton::areButtonsPressed(uint8_t btn1, uint8_t btn2);

// ?�스�??�식
GestureType RemoteButton::detectGesture();

// 커스?� ?�벤??
void RemoteButton::registerCustomEvent(CustomEventHandler handler);
```

## ?�� 코딩 규칙

1. **?�래???�름**: PascalCase (?? `RemoteLCD`)
2. **메서???�름**: camelCase (?? `showButtonStatus`)
3. **?�수**: UPPER_CASE (?? `BUTTON_COUNT`)
4. **멤버 변??*: camelCase with prefix (?? `currentTextSize`)
5. **?�일 ?�름**: ?�래???�름�??�일 (?? `RemoteLCD.h`, `RemoteLCD.cpp`)
