/*
 * ESP32 WROOM 리모컨 송신기 (ESP-NOW)
 * 
 * 기능:
 * - ESP-NOW를 사용하여 버튼 입력을 무선으로 전송
 * - TFT LCD 디스플레이로 상태 표시
 * - 12개 버튼 입력 지원 (PCA9555 I2C)
 * - 클래스 기반 구조화된 코드
 */

#include "class/lcd/RemoteLCD.h"
#include "class/button/RemoteButton.h"
#include "class/led/RemoteLED.h"
#include "class/espnow/RemoteESPNow.h"
#include "class/ybcar/YbCar.h"
#include "class/ybcarDoctor/YbCarDoctor.h"

// 수신기 MAC 주소 (실제 수신기의 MAC 주소로 변경 필요)
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// LCD, 버튼, LED, ESP-NOW, YbCar, YbCarDoctor 객체
RemoteLCD lcd;
RemoteButton buttons;
RemoteLED led;
RemoteESPNow espNow;
YbCar ybcar;
YbCarDoctor doctor;

// ESP-NOW 전송 결과 콜백
void onSendComplete(bool success) {
  if (success) {
    led.showSuccess();
  } else {
    led.showError();
  }
}

// 1초 주기 업데이트 콜백 (배터리 레벨, RSSI)
void onStatusUpdate(uint8_t batteryLevel, int8_t rssi) {
  lcd.showBatteryLevel(batteryLevel);
  lcd.showRSSI(rssi);
}

// ESP-NOW 데이터 수신 콜백 (차량 데이터 & 설정)
void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
  // 차량 데이터 크기 확인
  if (len == sizeof(vehicle_message)) {
    vehicle_message* vehicleData = (vehicle_message*)data;
    ybcar.updateVehicleData(vehicleData);
  }
  // 설정 메시지 크기 확인
  else if (len == sizeof(settings_message)) {
    settings_message* settingsData = (settings_message*)data;
    doctor.handleSettingsMessage(settingsData);
  }
  else {
    Serial.printf("수신 데이터 크기 불일치: %d bytes\n", len);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n=== ESP32 리모컨 시작 ===");
  
  // LED 초기화
  led.begin();
  
  // LCD 초기화
  Serial.println("LCD 초기화 중...");
  if (lcd.begin()) {
    lcd.drawMainScreen();
    Serial.println("LCD 초기화 성공");
  } else {
    Serial.println("LCD 초기화 실패!");
  }
  
  // 버튼 초기화 (PCA9555 I2C)
  Serial.println("버튼 초기화 중...");
  if (!buttons.begin()) {
    Serial.println("버튼 초기화 실패!");
    lcd.printTextCentered("버튼 초기화 실패!", 150, RemoteLCD::RED);
  } else {
    Serial.println("12개 버튼 준비 완료");
  }
  
  // 버튼 핸들러 설정 (LCD와 ESP-NOW 연결)
  buttons.setHandlers(&lcd, &espNow);
  
  // ESP-NOW 초기화
  Serial.println("ESP-NOW 초기화 중...");
  if (!espNow.begin()) {
    Serial.println("ESP-NOW 초기화 실패!");
    lcd.printTextCentered("ESP-NOW 초기화 실패!", 170, RemoteLCD::RED);
    return;
  }
  
  // LCD에 MAC 주소 표시
  lcd.setTextSize(1);
  String macStr = "MAC: " + espNow.getMacAddress();
  lcd.printText(macStr.c_str(), 10, 220, RemoteLCD::YELLOW);
  
  // 콜백 설정
  espNow.setSendCallback(onSendComplete);
  espNow.setUpdateCallback(onStatusUpdate);
  espNow.setReceiveCallback(onDataReceived);
  
  // YbCar 초기화
  ybcar.begin(&lcd, &espNow);
  
  // YbCarDoctor 초기화
  doctor.begin(&lcd, &espNow);
  
  // 수신기 설정
  if (!espNow.setReceiver(receiverAddress)) {
    Serial.println("수신기 설정 실패!");
    lcd.showConnectionStatus(false);
    return;
  }
  
  Serial.println("리모컨 준비 완료");
  Serial.println("수신기 MAC 주소를 설정하세요!");
  Serial.println("차량 데이터 수신 대기 중...");
  
  lcd.showConnectionStatus(true);
  
  // 차량 설정 요청 (5초 후)
  delay(5000);
  Serial.println("차량 설정 요청 전송...");
  doctor.requestSettings();
}

void loop() {
  // 버튼 스캔
  buttons.scan();
  
  // LED 업데이트 (깜박임 처리)
  led.update();
  
  // 이벤트 자동 처리 (RemoteButton 클래스에서 처리)
  buttons.processEvents();
  
  // ESP-NOW 업데이트 (1초 주기로 배터리 및 RSSI 업데이트)
  espNow.update();
  
  delay(10); // CPU 부하 감소
}
