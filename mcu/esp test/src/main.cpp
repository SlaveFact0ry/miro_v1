#include <Arduino.h>

// --- 핀 번호 설정 ---
#define TRIG_L 5
#define ECHO_L 18

#define TRIG_C 19
#define ECHO_C 23

#define TRIG_R 32
#define ECHO_R 33

// 최대 측정 대기 시간 (30ms = 약 5m)
const long TIMEOUT = 30000;

// 함수: 개별 센서 측정
float read_sonar(int trigPin, int echoPin, String name) {
  // 1. Trig 핀 초기화
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // 2. 발사!
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 3. 수신 대기
  long duration = pulseIn(echoPin, HIGH, TIMEOUT);
  
  if (duration == 0) {
    return -1.0; // 측정 실패
  } else {
    // cm 단위 변환
    return (duration * 0.034 / 2);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_L, OUTPUT); pinMode(ECHO_L, INPUT);
  pinMode(TRIG_C, OUTPUT); pinMode(ECHO_C, INPUT);
  pinMode(TRIG_R, OUTPUT); pinMode(ECHO_R, INPUT);
  
  Serial.println("========================================");
  Serial.println("Miro 로봇 초음파 3채널 순차 테스트 시작");
  Serial.println("========================================");
}

void loop() {
  float dist_L, dist_C, dist_R;

  // --- 1. 왼쪽 (Left) 측정 ---
  dist_L = read_sonar(TRIG_L, ECHO_L, "Left");
  delay(50); // 간섭 방지 대기 (50ms)

  // --- 2. 중앙 (Center) 측정 ---
  dist_C = read_sonar(TRIG_C, ECHO_C, "Center");
  delay(50); // 간섭 방지 대기 (50ms)

  // --- 3. 오른쪽 (Right) 측정 ---
  dist_R = read_sonar(TRIG_R, ECHO_R, "Right");
  
  // --- 결과 출력 ---
  Serial.print("L: ");
  if(dist_L == -1) Serial.print("---"); else Serial.print(dist_L, 1);
  Serial.print(" cm  |  ");

  Serial.print("C: ");
  if(dist_C == -1) Serial.print("---"); else Serial.print(dist_C, 1);
  Serial.print(" cm  |  ");

  Serial.print("R: ");
  if(dist_R == -1) Serial.println("---"); else Serial.println(dist_R, 1);

  delay(200); // 전체 루프 대기
}