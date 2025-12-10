# ESP32 WROOM 리모컨 프로젝트 (ESP-NOW + CAN)

ESP32 WROOM 모듈을 사용한 ESP-NOW 및 CAN 통신 기반 차량용 리모컨 프로젝트입니다.

## 📋 프로젝트 개요

이 프로젝트는 ESP-NOW와 CAN 통신을 함께 사용하는 양방향 무선 리모컨 시스템입니다.
- **ESP-NOW 통신**: 차량 제어 명령 전송 및 상태 수신
- **CAN 통신**: 차량 설정 모드 (16개 파라미터 조정)
- **5버튼 키패드**: 직관적인 메뉴 네비게이션
- **TFT LCD**: 실시간 차량 상태 및 설정 표시
- **클래스 기반 모듈 구조**: 7개 독립 클래스로 구성

## ✨ 주요 기능

### 🎮 리모컨 제어
- **5버튼 키패드 (12512WS-08)**: SELECT, UP, DOWN, LEFT, RIGHT
  - 디바운싱, 롱프레스, 더블클릭 지원
  - 특수 조합: **SELECT+LEFT+RIGHT (1초)** → 설정 모드 진입
- **TFT LCD (240x320, ST7789)**: 차량 상태 실시간 표시 (세로형)
  - **16x16 한글 조합형 폰트** 지원 (초성+중성+종성 분해)
  - 8x16 영문 폰트
  - 혼합 텍스트 렌더링 (UTF-8 파싱)
  - 📺 **[UI 디자인 보기](docs/ui-design.md)** | **[다이어그램 보기](docs/ui-diagram.md)**
- **상태 LED**: 통신 상태 시각 피드백

### 📡 이중 통신 시스템
- **ESP-NOW (2.4GHz WiFi)**:
  - 버튼 명령 전송 (전진/후진/좌우 제어)
  - 차량 데이터 수신 (속도, 배터리, 모터 온도, FET 온도, 전류)
  - RSSI 모니터링 (신호 강도)
  - WiFi 라우터 불필요
- **CAN 통신 (500kbps)**:
  - 차량 설정 모드 진입/종료
  - 16개 파라미터 실시간 조정
  - 설정값 요청/응답/업데이트

### 🚗 차량 설정 관리 (CAN)
1. **배터리/전력 설정**
   - 배터리 전압 (48V)
   - 전류 제한 (200A)
   - 저전압 임계값 (23V)
2. **온도 보호**
   - 모터 온도 제한 (90°C)
   - FET 온도 제한 (85°C)
3. **모터 제어**
   - 모터1/2 극성 설정
   - 전진/후진 속도 비율 (100%/80%)
   - 가속/감속 램프 (20/20)
4. **스로틀 설정**
   - 스로틀 오프셋 (300)
   - 변곡점 (900)
5. **브레이크 설정**
   - 브레이크 지연 (100ms)
   - 브레이크 레이트 (10)

## 🔧 하드웨어 구성

### 리모컨 (송신기)
- **MCU**: ESP32 WROOM (Dual-core, WiFi, BLE)
- **전원**: 3.3V
- **LCD**: SZH-EK096 (ST7789, 240x320, SPI) - 세로형
  - CS: GPIO 5
  - DC: GPIO 4
  - RST: GPIO 15
  - MOSI: GPIO 23
  - SCLK: GPIO 18
- **5버튼 키패드 (12512WS-08)**: 직접 GPIO 연결
  - SELECT: GPIO 12
  - DOWN: GPIO 13
  - RIGHT: GPIO 14
  - LEFT: GPIO 27
  - UP: GPIO 26
  - 모든 버튼: INPUT_PULLUP (LOW active)
- **CAN 트랜시버 (SN65HVD232)**: ESP32 내장 TWAI
  - TX: GPIO 21
  - RX: GPIO 22
  - 보드레이트: 500 kbps
- **상태 LED**: GPIO 2
- **UART**: RX, TX (프로그래밍 및 디버깅)

### 핀아웃 상세
자세한 하드웨어 연결은 [hardware-pinout.md](docs/hardware-pinout.md) 참고

## 📦 필요 사항

### 소프트웨어
- [PlatformIO](https://platformio.org/) (권장) 또는 Arduino IDE
- ESP32 보드 지원 패키지

### 하드웨어
- ESP32 WROOM 모듈 x 2 (송신기, 수신기)
- 택트 버튼 x 12
- LED x 1 (상태 표시용)
- 저항(LED용: 330Ω)
- USB-UART 변환기 (프로그래밍용)

## 🚀 시작하기

### 1. 프로젝트 빌드

```bash
# PlatformIO 사용 시
pio run

# 또는 VS Code에서 PlatformIO 확장 사용
```

### 2. 차량(수신기) MAC 주소 확인

1. 차량측 ESP32에서 MAC 주소 확인
2. 시리얼 모니터에서 MAC 주소 복사

```
차량 MAC 주소: XX:XX:XX:XX:XX:XX
```

### 3. 리모컨 설정

`src/main.cpp` 파일에서 차량 MAC 주소 설정:

```cpp
// 차량 수신기 MAC 주소 (실제 차량 ESP32의 MAC 주소로 변경)
uint8_t vehicleAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
espNow.setReceiver(vehicleAddress);
```

### 4. 리모컨 업로드

```bash
# PlatformIO 빌드 및 업로드
pio run --target upload

# 시리얼 모니터 열기
pio device monitor -b 115200
```

### 5. CAN 트랜시버 연결

SN65HVD232 CAN 트랜시버를 다음과 같이 연결:

```
ESP32 GPIO 21 → SN65HVD232 TX
ESP32 GPIO 22 ← SN65HVD232 RX
ESP32 3.3V    → SN65HVD232 VCC
ESP32 GND     → SN65HVD232 GND

SN65HVD232 CANH ↔ 차량 CAN_H (120Ω 종단 저항)
SN65HVD232 CANL ↔ 차량 CAN_L
```

## 📡 동작 원리

### 이중 통신 아키텍처

#### ESP-NOW 통신 (주 제어)
- **프로토콜**: ESP-NOW (Espressif 무선 프로토콜)
- **용도**: 실시간 차량 제어 및 상태 모니터링
- **특징**:
  - WiFi 라우터 불필요
  - 저지연 통신 (< 10ms)
  - 양방향 통신 (버튼 명령 송신, 차량 데이터 수신)
  - 최대 250 바이트 데이터
  - 통신 거리: 약 200m (장애물 없을 시)

#### CAN 통신 (설정 모드)
- **프로토콜**: CAN 2.0B (ISO 11898)
- **하드웨어**: ESP32 내장 TWAI 컨트롤러
- **용도**: 차량 파라미터 설정
- **특징**:
  - 500kbps 보드레이트
  - 산업 표준 프로토콜
  - 노이즈 내성 강함
  - 멀티마스터 지원
  - 통신 거리: 최대 40m @ 1Mbps, 수백m @ 500kbps

### 데이터 구조

#### ESP-NOW 메시지
```cpp
// 리모컨 → 차량 (버튼 명령)
struct ButtonMessage {
  uint8_t buttonId;      // 버튼 ID (0-4: SELECT/DOWN/RIGHT/LEFT/UP)
  uint8_t eventType;     // 이벤트 타입 (PRESS/RELEASE/LONG/DOUBLE)
  uint32_t timestamp;    // 타임스탬프
};

// 차량 → 리모컨 (상태 데이터)
struct VehicleData {
  float speed;           // 속도 (km/h)
  uint8_t direction;     // 방향 (0:정지, 1:전진, 2:후진)
  uint8_t batteryLevel;  // 배터리 레벨 (0-100%)
  float motorTemp;       // 모터 온도 (°C)
  float motorCurrent;    // 모터 전류 (A)
  float fetTemp;         // FET 온도 (°C)
  int8_t rssi;           // 신호 강도 (dBm)
};
```

#### CAN 메시지 ID
```cpp
// 설정 모드 제어
0x110: CAN_MODE_ENTER      // 설정 모드 진입
0x111: CAN_MODE_EXIT       // 설정 모드 종료
0x112: CAN_MODE_ACK        // 모드 변경 확인

// 설정 데이터
0x100: CAN_SETTINGS_REQUEST   // 설정값 요청
0x101: CAN_SETTINGS_RESPONSE  // 설정값 응답 (16개 파라미터)
0x102: CAN_SETTINGS_UPDATE    // 설정값 변경
```

## 🎮 사용법

### 일반 제어 모드 (ESP-NOW)
1. **전원 켜기**
   - LCD에 초기화 메시지 표시
   - LED 깜빡임: 준비 완료
2. **버튼 조작**
   - SELECT: 선택/확인
   - UP/DOWN/LEFT/RIGHT: 방향 제어
   - 버튼 누르면 ESP-NOW로 즉시 전송
3. **차량 상태 확인**
   - LCD에 실시간 표시: 속도, 배터리, 온도, RSSI
   - LED: 통신 상태 표시

### 설정 모드 진입 (CAN 통신)
1. **특수 조합 누르기**
   - SELECT + LEFT + RIGHT 동시에 1초 이상
   - LCD에 "설정 모드 진입..." 표시
2. **CAN 통신 시작**
   - MODE_ENTER (0x110) 메시지 전송
   - 차량이 ACK (0x112) 응답 대기
3. **설정 메뉴 탐색**
   - UP/DOWN: 항목 이동
   - LEFT/RIGHT: 값 조정
   - SELECT: 확인/저장
4. **설정 모드 종료**
   - SELECT 길게 누르기 (3초)
   - MODE_EXIT (0x111) 전송

### 차량 파라미터 설정 (16개 항목)
1. 배터리 전압 설정 (V)
2. 제한 전류 (A)
3. 제한 모터 온도 (°C)
4. 제한 FET 온도 (°C)
5. 저전압 임계값 (V)
6. Barity Im
7. 모터1 극성
8. 모터2 극성
9. 스로틀 오프셋
10. 스로틀 변곡점
11. 전진 비율 (%)
12. 후진 비율 (%)
13. 가속 램프
14. 감속 램프
15. 브레이크 지연 (ms)
16. 브레이크 레이트

## 🔨 커스터마이징

### 버튼 핀 변경
`src/class/button/RemoteButton.h` 수정:
```cpp
static const uint8_t PIN_BTN_SELECT = 12;  // 원하는 GPIO로 변경
static const uint8_t PIN_BTN_DOWN = 13;
// ...
```

### CAN 보드레이트 변경
`src/class/cancom/RemoteCANCom.cpp` 수정:
```cpp
// 500kbps → 250kbps로 변경 예시
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
```

### LCD 핀 변경
`src/class/lcd/RemoteLCD.h` 수정:
```cpp
static const uint8_t TFT_CS = 5;   // 원하는 GPIO
static const uint8_t TFT_DC = 4;
static const uint8_t TFT_RST = 15;
```

### 차량 설정 파라미터 추가
1. `src/class/ybcarDoctor/YbCarDoctor.h`에 변수 추가
2. CAN 메시지 처리 함수 업데이트
3. LCD 표시 함수에 UI 추가

### 전력 최적화
- Deep Sleep 사용 시:
  - GPIO 웨이크업 설정 (버튼 인터럽트)
  - ESP-NOW 재초기화 필요
- Light Sleep:
  - WiFi 모뎀 슬립 (esp_wifi_set_ps(WIFI_PS_MIN_MODEM))

## 🐛 문제 해결

### 5버튼 입력 안 됨
- **증상**: 버튼 눌러도 반응 없음
- **확인 사항**:
  - GPIO 연결 확인 (SELECT=12, DOWN=13, RIGHT=14, LEFT=27, UP=26)
  - 공통 GND 연결 확인
  - INPUT_PULLUP 설정 확인 (내부 풀업 사용)
  - 버튼 스위치 정상 동작 확인 (멀티미터로 단락 테스트)

### ESP-NOW 초기화 실패
- **증상**: "ESP-NOW 초기화 실패" 메시지
- **해결**:
  - WiFi 모드 확인 (WIFI_MODE_STA)
  - ESP32 보드 리셋
  - 다른 WiFi 라이브러리와 충돌 확인
  - 펌웨어 재업로드

### CAN 통신 안 됨
- **증상**: 설정 모드 진입 안 됨
- **확인 사항**:
  - SN65HVD232 트랜시버 연결 (TX=21, RX=22)
  - CAN_H, CAN_L 배선 확인 (트위스트 페어)
  - 120Ω 종단 저항 설치 (CAN 버스 양 끝)
  - 차량측 CAN 컨트롤러 동작 확인
  - CAN 보드레이트 일치 (500kbps)
  - 시리얼 모니터에서 CAN 메시지 로그 확인

### LCD 표시 안 됨
- **증상**: LCD 화면 깜깜함 또는 노이즈
- **확인 사항**:
  - SPI 핀 연결 (CS=5, DC=4, RST=15, MOSI=23, SCLK=18)
  - 3.3V 전원 공급 확인
  - 백라이트 연결 확인
  - RST 핀 동작 확인
  - Adafruit 라이브러리 버전 확인

### 차량 데이터 수신 안 됨
- **증상**: LCD에 차량 상태 표시 안 됨, RSSI: 0
- **확인 사항**:
  - 차량 MAC 주소 올바르게 설정
  - 차량측 ESP-NOW 송신 코드 동작 확인
  - 통신 거리 (최대 200m)
  - 장애물 확인
  - RSSI 값 모니터링 (시리얼 출력)

### 메모리 부족
- **증상**: 빌드 실패 또는 런타임 크래시
- **해결**:
  - PSRAM 활성화 (platformio.ini)
  - LCD 버퍼 크기 최적화
  - 사용하지 않는 클래스 제거
  - 로그 레벨 낮추기 (esp_log_level_set)

### 버튼이 반응하지 않음
- PCA9555 I2C 통신 확인
- 버튼 연결 확인 (IOI 핀 - GND)
- 시리얼 모니터로 버튼 입력 확인

## 📚 참고 자료

### 통신 프로토콜
- [ESP-NOW 공식 문서](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [ESP32 TWAI (CAN) 드라이버](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html)
- [CAN 2.0B 사양](https://www.can-cia.org/can-knowledge/)

### 하드웨어
- [ESP32 핀아웃 참조](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ST7789 TFT LCD 데이터시트](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)
- [SN65HVD232 CAN 트랜시버](https://www.ti.com/product/SN65HVD232)
- [12512WS-08 5버튼 키패드](docs/12512ws-08_5button_spec.md)

### 개발 도구
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [PlatformIO ESP32 가이드](https://docs.platformio.org/en/latest/platforms/espressif32.html)
- [Adafruit GFX 라이브러리](https://learn.adafruit.com/adafruit-gfx-graphics-library)

### 프로젝트 문서
- [하드웨어 핀 맵핑](docs/hardware-pinout.md)
- [클래스 구조 설명](docs/class-structure.md)
- [5버튼 키패드 사양](docs/12512ws-08_5button_spec.md)

## 📁 프로젝트 구조

```
esp32_core_remocon/
├── src/
│   ├── main.cpp                          # 메인 프로그램
│   └── class/
│       ├── lcd/                          # LCD 디스플레이 클래스
│       │   ├── RemoteLCD.h
│       │   └── RemoteLCD.cpp
│       ├── button/                       # 5버튼 입력 클래스
│       │   ├── RemoteButton.h
│       │   └── RemoteButton.cpp
│       ├── led/                          # 상태 LED 클래스
│       │   ├── RemoteLED.h
│       │   └── RemoteLED.cpp
│       ├── espnow/                       # ESP-NOW 통신 클래스
│       │   ├── RemoteESPNow.h
│       │   └── RemoteESPNow.cpp
│       ├── cancom/                       # CAN 통신 클래스 (ESP32 TWAI)
│       │   ├── RemoteCANCom.h
│       │   └── RemoteCANCom.cpp
│       ├── ybcar/                        # 차량 데이터 관리 클래스
│       │   ├── YbCar.h
│       │   └── YbCar.cpp
│       └── ybcarDoctor/                  # 차량 설정 관리 클래스
│           ├── YbCarDoctor.h
│           └── YbCarDoctor.cpp
├── docs/
│   ├── hardware-pinout.md                # 하드웨어 핀 맵핑
│   ├── 12512ws-08_5button_spec.md        # 5버튼 키패드 사양
│   └── class-structure.md                # 클래스 구조 설명
├── platformio.ini                        # PlatformIO 설정
└── README.md                             # 프로젝트 설명
```

### 클래스 구조

#### 1. RemoteLCD (LCD 디스플레이)
- TFT LCD 초기화 및 제어
- 차량 상태 표시 (속도, 배터리, 온도, RSSI)
- 버튼 상태 표시
- 설정 메뉴 UI

#### 2. RemoteButton (5버튼 입력)
- GPIO 직접 입력 (INPUT_PULLUP)
- 디바운싱, 롱프레스, 더블클릭
- 특수 조합 감지 (SELECT+LEFT+RIGHT → 설정 모드)
- 이벤트 핸들러

#### 3. RemoteLED (상태 LED)
- LED 제어 (켜기/끄기/깜빡임)
- 성공/에러 패턴 표시

#### 4. RemoteESPNow (ESP-NOW 통신)
- 양방향 ESP-NOW 통신
- 버튼 명령 전송
- 차량 데이터 수신
- RSSI 모니터링

#### 5. RemoteCANCom (CAN 통신)
- ESP32 내장 TWAI 드라이버 사용
- 설정 모드 진입/종료
- 설정값 요청/응답/업데이트
- CAN 메시지 처리

#### 6. YbCar (차량 데이터)
- 차량 상태 데이터 관리
- ESP-NOW로 수신한 데이터 저장
- LCD 디스플레이 업데이트

#### 7. YbCarDoctor (차량 설정)
- 16개 파라미터 관리
- CAN 통신으로 설정 송수신
- Preferences를 사용한 EEPROM 저장
- 설정 메뉴 UI 제어

## 📄 라이선스

MIT License

## 👤 작성자

프로젝트 작성일: 2025-12-09

## 📝 버전 히스토리

### v2.1.0 (2025-12-10) - UI Redesign & Korean Font
- **UI 개선**
  - 5버튼 십자 레이아웃으로 재설계
  - 16x16 한글 조합형 폰트 통합 (~11KB)
  - UTF-8 혼합 텍스트 렌더링 (한글+영문)
  - 차량 상태 표시 영역 재배치
  - 📺 **[UI 디자인 문서](docs/ui-design.md)** | **[다이어그램](docs/ui-diagram.md)**
- **폰트 시스템**
  - `draw16String()`: UTF-8 문자열 렌더링
  - `draw16Korean()`: 초성(20) + 중성(22) + 종성(28) 조합
  - `draw16English()`: 8x16 ASCII 렌더링
  - 스케일링 및 투명 배경 지원

### v2.0.0 (2025-12-09) - Major Refactor
- **하드웨어 변경**
  - 12버튼 PCA9555 I2C → 5버튼 직접 GPIO (12512WS-08)
  - 외부 MCP2515 CAN → ESP32 내장 TWAI CAN
- **CAN 통신 추가**
  - RemoteCANCom 클래스 구현
  - 16개 차량 파라미터 설정 기능
  - 설정 모드 진입 조합 (SELECT+LEFT+RIGHT, 1초)
- **이중 통신 아키텍처**
  - ESP-NOW: 실시간 차량 제어
  - CAN: 차량 설정 관리
- **핀 맵핑 변경**
  - GPIO 21/22: I2C → CAN TX/RX
  - GPIO 12/13/14/26/27: 5버튼 직접 입력

### v1.0.0 (2025-12-08) - Initial Release
- 12버튼 PCA9555 I2C 리모컨
- ESP-NOW 통신
- TFT LCD 디스플레이 (ST7789)
- 클래스 기반 모듈 구조
- YbCar 차량 데이터 관리
- YbCarDoctor 차량 설정 관리

## 🔮 향후 개선 사항

- [ ] 배터리 전압 ADC 모니터링 (리모컨 자체 배터리)
- [ ] Deep Sleep 모드 구현 (버튼 웨이크업)
- [ ] CAN 설정 UI 완성 (네비게이션, 값 조정)
- [ ] OTA 무선 펌웨어 업데이트
- [ ] BLE 추가 (스마트폰 연동)
- [ ] 암호화 통신 (ESP-NOW 암호화)
- [ ] 다중 차량 지원 (차량 ID 선택)
- [ ] 로그 기록 (SD 카드 또는 SPIFFS)
- [ ] CAN 진단 기능 (DTC 코드 읽기)
- [ ] 차량 블랙박스 데이터 다운로드
