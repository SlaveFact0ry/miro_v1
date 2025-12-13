#include <Arduino.h>
#include <ESP32Servo.h>

// --- 핀 설정 ---
#define PIN_MOTOR_L 12
#define PIN_MOTOR_R 14

Servo motL;
Servo motR;

// --- 설정 변수 ---
int current_us = 1500; // 초기값 (중립)

void setup() {
  Serial.begin(115200);
  
  // ESP32Servo 설정
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // 모터 핀 연결 (기본 범위 1000~2000us)
  motL.setPeriodHertz(50); // 표준 50Hz PWM
  motR.setPeriodHertz(50);
  
  motL.attach(PIN_MOTOR_L, 1000, 2000);
  motR.attach(PIN_MOTOR_R, 1000, 2000);

  // 초기화 시 안전하게 중립(1500) 전송
  motL.writeMicroseconds(1500);
  motR.writeMicroseconds(1500);

  Serial.println("=========================================");
  Serial.println("   Miro Motor ESC Calibrator & Tester    ");
  Serial.println("=========================================");
  Serial.println("사용법:");
  Serial.println(" - 숫자 입력 (예: 1500): 해당 us 값 전송");
  Serial.println(" - '+' 키: +10us 증가");
  Serial.println(" - '-' 키: -10us 감소");
  Serial.println(" - 'm' 키: [MAX] 2000us 전송 (캘리브레이션용)");
  Serial.println(" - 'n' 키: [MIN] 1000us 전송 (캘리브레이션용)");
  Serial.println(" - 's' 키: [STOP] 1500us 전송 (중립)");
  Serial.println("-----------------------------------------");
  Serial.print("현재 Pulse: "); Serial.println(current_us);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.peek(); // 첫 글자 확인

    // 숫자가 들어오면 값으로 읽음
    if (isDigit(c)) {
      int val = Serial.parseInt();
      if (val >= 800 && val <= 2200) {
        current_us = val;
      } else {
        Serial.println("범위 초과! (800~2200 사이 입력)");
      }
      // 버퍼 비우기 (엔터키 처리)
      while(Serial.available()) Serial.read(); 
    } 
    // 문자 명령 처리
    else {
      char cmd = Serial.read();
      switch (cmd) {
        case '+': current_us += 10; break;
        case '-': current_us -= 10; break;
        case 'm': current_us = 2000; Serial.println(">> MAX THROTTLE (2000)"); break;
        case 'n': current_us = 1000; Serial.println(">> MIN THROTTLE (1000)"); break;
        case 's': current_us = 1500; Serial.println(">> STOP / NEUTRAL (1500)"); break;
      }
      // 줄바꿈 문자 무시
      while(Serial.available() && Serial.peek() == '\n') Serial.read(); 
    }

    // 모터에 값 적용 (L, R 동시 적용)
    motL.writeMicroseconds(current_us);
    motR.writeMicroseconds(current_us);

    Serial.print("Output: "); Serial.print(current_us); Serial.println(" us");
  }
}