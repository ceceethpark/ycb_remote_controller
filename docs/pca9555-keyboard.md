# PCA9555 I2C ?¤ë³´???¤ì • ê°€?´ë“œ

## ?“‹ ê°œìš”

ESP32 ë¦¬ëª¨ì»¨ì? PCA9555 I2C GPIO ?•ì¥ ì¹©ì„ ?¬ìš©?˜ì—¬ 12ê°œì˜ ë²„íŠ¼ ?…ë ¥??ì²˜ë¦¬?©ë‹ˆ??

## ?”Œ ?˜ë“œ?¨ì–´ ?°ê²°

### PCA9555 ?€ ?°ê²°
```
ESP32          PCA9555
GPIO 21 (SDA) ??SDA
GPIO 22 (SCL) ??SCL
3.3V          ??VCC
GND           ??GND
              ??A0 (GND)  // ì£¼ì†Œ ?¤ì •
              ??A1 (GND)  // ì£¼ì†Œ ?¤ì •
              ??A2 (GND)  // ì£¼ì†Œ ?¤ì •
```

### I2C ì£¼ì†Œ
- **ê¸°ë³¸ ì£¼ì†Œ**: 0x20 (A0, A1, A2 ëª¨ë‘ GND)
- ?¤ë¥¸ ì£¼ì†Œê°€ ?„ìš”??ê²½ìš° A0~A2 ?€?¼ë¡œ ?¤ì • ê°€??

### ë²„íŠ¼ ?°ê²° (12ê°?
```
PCA9555 ?¬íŠ¸    ë²„íŠ¼ ID    ?°ê²°
IOI_0          0         ë²„íŠ¼ 1
IOI_1          1         ë²„íŠ¼ 2
IOI_2          2         ë²„íŠ¼ 3
IOI_3          3         ë²„íŠ¼ 4
IOI_4          4         ë²„íŠ¼ 5
IOI_5          5         ë²„íŠ¼ 6
IOI_6          6         ë²„íŠ¼ 7
IOI_7          7         ë²„íŠ¼ 8
IOI_8          8         ë²„íŠ¼ 9
IOI_9          9         ë²„íŠ¼ 10
IOI_10         10        ë²„íŠ¼ 11
IOI_11         11        ë²„íŠ¼ 12
```

ê°?ë²„íŠ¼?€ PCA9555 IOI ?€ê³?GND ?¬ì´???°ê²° (?´ë? ?€???¬ìš©)

## ?“Š ?Œë¡œ???•ë³´

### PCA9555 ?ˆì??¤í„°
- **INPUT_PORT_0 (0x00)**: IOI_0 ~ IOI_7 ?…ë ¥ ?íƒœ
- **INPUT_PORT_1 (0x01)**: IOI_8 ~ IOI_15 ?…ë ¥ ?íƒœ
- **CONFIG_PORT_0 (0x06)**: Port 0 ë°©í–¥ ?¤ì • (1=?…ë ¥)
- **CONFIG_PORT_1 (0x07)**: Port 1 ë°©í–¥ ?¤ì • (1=?…ë ¥)

### ?µì‹  ?¤ì •
- **I2C ì£¼íŒŒ??*: 400kHz (Fast Mode)
- **ë²„ìŠ¤ ?„ì••**: 3.3V
- **?€???€??*: 4.7kÎ© (SDA, SCL)

## ?’» ?Œí”„?¸ì›¨???¤ì •

### RemoteButton ?´ë˜???¬ìš©

```cpp
#include "class/button/RemoteButton.h"

RemoteButton keyboard;

void setup() {
    // ?¤ë³´??ì´ˆê¸°??
    if (!keyboard.begin()) {
        Serial.println("PCA9555 ì´ˆê¸°???¤íŒ¨!");
        return;
    }
    Serial.println("12???¤ë³´??ì¤€ë¹??„ë£Œ");
}

void loop() {
    // ë²„íŠ¼ ?¤ìº”
    keyboard.scan();
    
    // ?´ë²¤??ì²˜ë¦¬
    while (keyboard.hasEvent()) {
        ButtonEventInfo event = keyboard.getEvent();
        
        Serial.print("ë²„íŠ¼ ");
        Serial.print(event.buttonId);
        Serial.print(" - ");
        
        switch (event.event) {
            case BUTTON_PRESSED:
                Serial.println("?Œë¦¼");
                break;
            case BUTTON_RELEASED:
                Serial.println("ë¦´ë¦¬??);
                break;
            case BUTTON_LONG_PRESS:
                Serial.println("ë¡±í”„?ˆìŠ¤");
                break;
        }
    }
}
```

### ë²„íŠ¼ ID ?ìˆ˜
```cpp
keyboard.BUTTON_1   // 0  (IOI_0)
keyboard.BUTTON_2   // 1  (IOI_1)
keyboard.BUTTON_3   // 2  (IOI_2)
keyboard.BUTTON_4   // 3  (IOI_3)
keyboard.BUTTON_5   // 4  (IOI_4)
keyboard.BUTTON_6   // 5  (IOI_5)
keyboard.BUTTON_7   // 6  (IOI_6)
keyboard.BUTTON_8   // 7  (IOI_7)
keyboard.BUTTON_9   // 8  (IOI_8)
keyboard.BUTTON_10  // 9  (IOI_9)
keyboard.BUTTON_11  // 10 (IOI_10)
keyboard.BUTTON_12  // 11 (IOI_11)
```

## ?¨ LCD ?”ìŠ¤?Œë ˆ??

12ê°?ë²„íŠ¼?€ LCD??3??4?´ë¡œ ?œì‹œ?©ë‹ˆ??

```
?Œâ??€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€??
??     ESP32 ë¦¬ëª¨ì»?            ??
?œâ??€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€??
??[K0] [K1] [K2] [K3]           ??
??[K4] [K5] [K6] [K7]           ??
??[K8] [K9] [K10] [K11]         ??
?œâ??€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€??
??ë°°í„°ë¦? 85%    ?°ê²°: ??       ??
??PCA9555 12??ë¦¬ëª¨ì»?          ??
?”â??€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€?€??
```

## ?”§ ?¤ì • ë³€ê²?

### I2C ì£¼ì†Œ ë³€ê²?
`RemoteButton.h`?ì„œ:
```cpp
static const uint8_t PCA9555_ADDRESS = 0x20;  // ?í•˜??ì£¼ì†Œë¡?ë³€ê²?
```

### I2C ?€ ë³€ê²?
`RemoteButton.h`?ì„œ:
```cpp
static const uint8_t I2C_SDA = 21;  // ?¤ë¥¸ ?€?¼ë¡œ ë³€ê²?
static const uint8_t I2C_SCL = 22;  // ?¤ë¥¸ ?€?¼ë¡œ ë³€ê²?
```

### ?”ë°”?´ìŠ¤ ?œê°„ ë³€ê²?
```cpp
keyboard.setDebounceTime(50);      // 50ms (ê¸°ë³¸ê°?
keyboard.setLongPressTime(1000);   // 1ì´?(ê¸°ë³¸ê°?
```

## ?› ë¬¸ì œ ?´ê²°

### I2C ?µì‹  ?¤ë¥˜
```
PCA9555 ì´ˆê¸°???¤íŒ¨ - I2C ?µì‹  ?¤ë¥˜
```
**?´ê²° ë°©ë²•**:
1. I2C ?°ê²° ?•ì¸ (SDA, SCL)
2. ?€???€???•ì¸ (4.7kÎ©)
3. PCA9555 ?„ì› ?•ì¸ (3.3V)
4. I2C ì£¼ì†Œ ?•ì¸ (A0, A1, A2 ?¤ì •)

### ë²„íŠ¼??ë°˜ì‘?˜ì? ?ŠìŒ
1. ë²„íŠ¼ ?°ê²° ?•ì¸ (IOI ?€ - GND)
2. ?œë¦¬??ëª¨ë‹ˆ?°ì—???…ë ¥ ?•ì¸
3. I2C ?¤ìº?ˆë¡œ PCA9555 ì£¼ì†Œ ?•ì¸

### I2C ?¤ìº??ì½”ë“œ
```cpp
void scanI2C() {
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("I2C ?¥ì¹˜ ë°œê²¬: 0x");
            Serial.println(addr, HEX);
        }
    }
}
```

## ?“š PCA9555 ?°ì´?°ì‹œ??
- ?œì¡°?? Texas Instruments / NXP
- 16ë¹„íŠ¸ I2C GPIO ?•ì¥ê¸?
- I2C Fast Mode ì§€??(400kHz)
- ?…ë ¥ ?€??ì§€??

## ?”— ì¶”ê? ?•ë³´
- [PCA9555 ?°ì´?°ì‹œ??(https://www.ti.com/product/PCA9555)
- [I2C ?„ë¡œ? ì½œ](https://www.i2c-bus.org/)
