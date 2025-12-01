#include <Arduino.h>

// Center 센서 핀 설정
#define TRIG_PIN 19
#define ECHO_PIN 23

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println("===============================");
  Serial.println("JSN-SR04T 초음파 센서 테스트");
  Serial.println("===============================");
}

void loop() {
  // 1. Trig 핀 초기화 (Low 상태 유지)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // 2. 10us 펄스 발사 (야! 하고 소리지름)
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // 3. Echo 핀으로 돌아오는 시간 측정 (타임아웃 30ms = 약 5m)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) {
    Serial.println("거리 측정 실패 (0cm) - 배선 확인 필요");
  } else {
    // 소리 속도 340m/s. 왕복이므로 /2. cm 환산
    float distance = duration * 0.034 / 2;
    
    Serial.print("감지된 거리: ");
    Serial.print(distance);
    Serial.println(" cm");
  }
  
  delay(500); // 0.5초마다 측정
}