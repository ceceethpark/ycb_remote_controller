# ESP32 리모컨 프로젝트 - 업데이트 노트

## 2025-12-09 업데이트

### ✨ 주요 변경사항

#### 1. 클래스 기반 구조로 전환
- LCD 기능을 `RemoteLCD` 클래스로 분리
- 키보드 기능을 `RemoteKeyboard` 클래스로 분리
- 직관적인 폴더 구조 및 네이밍

#### 2. LCD 디스플레이 추가
- **모델**: SZH-EK096 (ST7789, 320x240)
- **통신**: SPI 4선식
- **기능**:
  - 메인 화면 UI
  - 버튼 상태 표시
  - 연결 상태 표시
  - 배터리 레벨 표시
  - 진행바, 텍스트, 버튼 그리기

#### 3. 고급 키보드 기능
- 디바운싱 (50ms)
- 롱프레스 감지 (1초)
- 더블클릭 지원 (300ms)
- 이벤트 큐 시스템
- 설정 가능한 타이밍

### 📁 새로운 파일 구조

```
esp32_core_remocon/
├── include/class/
│   ├── lcd/RemoteLCD.h
│   └── keyboard/RemoteKeyboard.h
├── src/class/
│   ├── lcd/RemoteLCD.cpp
│   └── keyboard/RemoteKeyboard.cpp
└── docs/
    └── class-structure.md
```

### 🔧 설정 변경

#### platformio.ini
```ini
lib_deps = 
    adafruit/Adafruit GFX Library @ ^1.11.9
    adafruit/Adafruit ST7735 and ST7789 Library @ ^1.10.3
    adafruit/Adafruit BusIO @ ^1.15.0
```

### 🎯 사용법

#### LCD 사용
```cpp
RemoteLCD lcd;

lcd.begin();
lcd.drawMainScreen();
lcd.showButtonStatus(1, true);
lcd.showConnectionStatus(true);
lcd.showBatteryLevel(85);
```

#### 키보드 사용
```cpp
RemoteKeyboard keyboard;

keyboard.begin();
keyboard.scan();

while (keyboard.hasEvent()) {
    ButtonEventInfo event = keyboard.getEvent();
    // 이벤트 처리
}
```

### 🐛 버그 수정
- 버튼 디바운싱 개선
- 메모리 누수 방지
- 안정적인 ESP-NOW 통신

### 📚 추가 문서
- `docs/class-structure.md` - 클래스 구조 상세 설명
- 코드 주석 개선
- 예제 코드 추가

### 🔜 향후 계획
- [ ] 배터리 전압 ADC 측정
- [ ] 설정 메뉴 시스템
- [ ] Wi-Fi 설정 UI
- [ ] 저전력 모드 (Deep Sleep)
- [ ] OTA 업데이트 지원
