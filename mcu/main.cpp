#include <Arduino.h>
#include <MPU9250.h>

MPU9250 mpu(Wire, 0x68); // 만약 스캐너에서 0x69 나오면 여기 수정

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.print("MPU9250 초기화 중...");
  int status = mpu.begin();
  
  if (status < 0) {
    Serial.println("실패! (에러코드 확인 필요)");
    while(1) {}
  }
  Serial.println("성공!");
}

void loop() {
  mpu.readSensor();
  
  // 가속도(Accel)와 자이로(Gyro) 값이 변하는지 확인
  Serial.print("Accel Z: "); Serial.print(mpu.getAccelZ_mss(), 2);
  Serial.print("\tGyro Z: "); Serial.println(mpu.getGyroZ_rads(), 2);
  
  delay(100);
}
