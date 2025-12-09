# ESP32 WROOM 리모컨 프로젝트 (ESP-NOW)

ESP32 WROOM 모듈을 사용한 ESP-NOW 기반 무선 리모컨 프로젝트입니다.

## 📋 프로젝트 개요

이 프로젝트는 ESP-NOW 프로토콜을 사용하여 버튼 입력을 무선으로 전송하는 리모컨 시스템입니다.
- **송신기(리모컨)**: 버튼 입력을 감지하고 ESP-NOW로 전송 + TFT LCD 디스플레이
- **수신기**: 신호를 수신하여 원하는 동작 수행
- **클래스 기반 구조**: LCD와 버튼을 독립적인 클래스로 분리

## ✨ 주요 기능

- 🔘 **12버튼 입력 (PCA9555 I2C)**: 디바운싱, 롱프레스, 더블클릭 지원
- 📺 **TFT LCD 디스플레이**: 실시간 상태 표시 (320x240 해상도)
- 📡 **ESP-NOW 무선 통신**: WiFi 라우터 없이 직접 통신
- 🔋 **배터리 모니터링**: 배터리 레벨 표시
- 🎨 **직관적인 UI**: 12개 버튼 상태를 3x4 그리드로 표시

## 🔧 하드웨어 구성

### 리모컨(송신기)
- **MCU**: ESP32 WROOM
- **전원**: VCC 3.3V
- **LCD**: SZH-EK096 (ST7789, 320x240, SPI 4선)
  - CS: GPIO 5
  - DC: GPIO 4
  - RST: GPIO 15
  - MOSI: GPIO 23
  - SCLK: GPIO 18
- **키보드**: PCA9555 I2C GPIO 확장 (12개 버튼)
  - I2C SDA: GPIO 21
  - I2C SCL: GPIO 22
  - 주소: 0x20
  - 버튼: IOI_0 ~ IOI_11 (12개)
- **상태 LED**: GPIO 2
- **UART**: RX, TX (프로그래밍 및 디버깅)

### 핀아웃 참고
첨부된 핀아웃을 참고하여 연결하세요.

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

### 2. 수신기 MAC 주소 확인

1. `examples/receiver.cpp`를 별도 ESP32에 업로드
2. 시리얼 모니터 열기 (115200 baud)
3. 출력되는 MAC 주소 복사

```
수신기 MAC 주소: XX:XX:XX:XX:XX:XX
이 주소를 리모컨 코드에 입력하세요
```

### 3. 리모컨 설정

`src/main.cpp` 파일에서 수신기 MAC 주소를 설정:

```cpp
// 수신기 MAC 주소 (실제 수신기의 MAC 주소로 변경)
uint8_t receiverAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
```

### 4. 리모컨 업로드

```bash
# PlatformIO 사용 시
pio run --target upload

# 시리얼 모니터 열기
pio device monitor
```

## 📡 동작 원리

### ESP-NOW 통신
- **프로토콜**: ESP-NOW (Espressif의 무선 통신 프로토콜)
- **특징**:
  - WiFi 연결 불필요
  - 저지연 통신 (< 10ms)
  - 저전력
  - 최대 250 바이트 데이터 전송
  - 최대 통신 거리: 약 200m (장애물 없을 시)

### 데이터 구조

```cpp
typedef struct struct_message {
  uint8_t buttonId;      // 버튼 번호 (0-11)
  uint8_t buttonState;   // 버튼 상태 (0: 릴리스, 1: 눌림)
  uint32_t timestamp;    // 타임스탬프 (디버깅용)
} struct_message;
```

## 🎮 사용법

### 리모컨(송신기)
1. 전원 연결
2. LED가 깜빡이면 준비 완료
3. 버튼 누르기
4. 전송 성공 시 LED 짧게 켜짐

### 수신기
1. 수신기 코드 업로드 및 실행
2. 리모컨 버튼 입력 대기
3. 수신된 데이터에 따라 동작 수행

## 🔨 커스터마이징

### 버튼 추가
1. `main.cpp`의 버튼 수 상수 수정
2. `loop()` 함수의 버튼 체크 코드 추가
3. 수신기에 해당 버튼 처리 로직 추가

### 전송 데이터 변경
1. `struct_message` 구조체 수정
2. 송신기와 수신기 모두 동일하게 수정 필요

### 전력 최적화
- Deep Sleep 모드 사용
- 버튼 인터럽트 사용
- WiFi 전력 관리 설정

## 🐛 문제 해결

### PCA9555 초기화 실패
- I2C 연결 확인 (SDA: GPIO 21, SCL: GPIO 22)
- 풀업 저항 확인 (4.7kΩ)
- PCA9555 전원 확인 (3.3V)
- I2C 주소 확인 (기본값: 0x20)

### ESP-NOW 초기화 실패
- WiFi 모드가 올바르게 설정되었는지 확인
- ESP32 보드 리셋

### 데이터 전송 실패
- 수신기 MAC 주소가 올바른지 확인
- 두 기기 간 거리가 너무 멀지 않은지 확인
- 수신기가 정상 동작 중인지 확인

### 버튼이 반응하지 않음
- PCA9555 I2C 통신 확인
- 버튼 연결 확인 (IOI 핀 - GND)
- 시리얼 모니터로 버튼 입력 확인

## 📚 참고 자료

- [ESP-NOW 공식 문서](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [PlatformIO ESP32 가이드](https://docs.platformio.org/en/latest/platforms/espressif32.html)

## 📄 라이선스

MIT License

## 👤 작성자

프로젝트 작성일: 2025-12-09

## 📝 버전 히스토리

- v1.0.0 - 초기 릴리스
  - 12버튼 리모컨 기본 기능
  - ESP-NOW 통신
  - TFT LCD 디스플레이
  - 클래스 기반 구조
  - YbCar 차량 데이터 관리
  - YbCarDoctor 차량 설정 관리

## 🔮 향후 개선 사항

- [ ] 배터리 전압 모니터링
- [ ] Deep Sleep 모드 구현
- [ ] 버튼 조합 기능
- [ ] 암호화 통신
- [ ] 다중 수신기 지원
- [ ] 설정 메뉴 UI
