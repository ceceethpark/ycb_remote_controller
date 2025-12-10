/*
 * ESP32 WROOM 리모컨 송신기
 * 
 * 기능:
 * - 5버튼 입력 지원 (12512WS-08: SELECT, UP, DOWN, LEFT, RIGHT)
 * - ESP-NOW 무선 통신
 * - CAN 통신 (차량 설정)
 * - TFT LCD 디스플레이
 * - 설정 모드: SELECT + LEFT + RIGHT 동시 누름 (1초)
 */

#include "class/lcd/RemoteLCD.h"
#include "class/button/RemoteButton.h"
#include "class/led/RemoteLED.h"
#include "class/espnow/RemoteESPNow.h"
#include "class/cancom/RemoteCANCom.h"
#include "class/ybcar/YbCar.h"
#include "class/ybcarDoctor/YbCarDoctor.h"

// 수신기 MAC 주소 (실제 수신기의 MAC 주소로 변경 필요)
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// 객체 생성
RemoteLCD lcd;
RemoteButton buttons;
RemoteLED led;
RemoteESPNow espNow;
RemoteCANCom canCom;
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
    printf("수신 데이터 크기 불일치: %d bytes\r\n", len);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  printf("\r\n=== ESP32 리모컨 시작 ===\r\n");
  
  // LED 초기화
  led.begin();
  
  // LCD 초기화
  printf("LCD 초기화 중...\r\n");
  if (lcd.begin()) {
    lcd.drawMainScreen();
    printf("LCD 초기화 성공\r\n");
  } else {
    printf("LCD 초기화 실패!\r\n");
  }
  
  // 버튼 초기화 (12512WS-08 5버튼)
  printf("버튼 초기화 중...\r\n");
  if (!buttons.begin()) {
    printf("버튼 초기화 실패!\r\n");
    lcd.printTextCentered("버튼 초기화 실패!", 150, RemoteLCD::RED);
  } else {
    printf("5개 버튼 준비 완료\r\n");
  }
  
  // CAN 통신 초기화 (ESP32 내장 CAN)
  printf("CAN 통신 초기화 중...\r\n");
  if (!canCom.begin(GPIO_NUM_21, GPIO_NUM_22)) {  // TX=GPIO21, RX=GPIO22
    printf("CAN 초기화 실패!\r\n");
    lcd.printTextCentered("CAN 초기화 실패", 170, RemoteLCD::YELLOW);
  } else {
    printf("ESP32 내장 CAN 준비 완료\r\n");
    lcd.setTextSize(1);
    lcd.printText("CAN: 500kbps", 200, 220, RemoteLCD::CYAN);
  }
  
  // 핸들러 설정
  buttons.setHandlers(&lcd, &espNow, &canCom);
  canCom.setHandlers(&lcd, &doctor);
  
  // ESP-NOW 초기화
  printf("ESP-NOW 초기화 중...\r\n");
  if (!espNow.begin()) {
    printf("ESP-NOW 초기화 실패!\r\n");
    lcd.printTextCentered("ESP-NOW 초기화 실패!", 185, RemoteLCD::RED);
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
    printf("수신기 설정 실패!\r\n");
    lcd.showConnectionStatus(false);
    return;
  }
  
  printf("리모컨 준비 완료\r\n");
  printf("수신기 MAC 주소를 설정하세요!\r\n");
  printf("차량 데이터 수신 대기 중...\r\n");
  
  lcd.showConnectionStatus(true);
  
  // 차량 설정 요청 (5초 후)
  delay(5000);
  printf("차량 설정 요청 전송...\r\n");
  doctor.requestSettings();
}

void loop() {
  // 버튼 스캔 (설정 모드 콤보 체크 포함)
  buttons.scan();
  
  // LED 업데이트 (깜박임 처리)
  led.update();
  
  // 버튼 이벤트 자동 처리
  buttons.processEvents();
  
  // ESP-NOW 업데이트 (1초 주기로 배터리 및 RSSI 업데이트)
  espNow.update();
  
  // CAN 통신 업데이트 (메시지 수신 처리)
  canCom.update();
  
  delay(10); // CPU 부하 감소
}
