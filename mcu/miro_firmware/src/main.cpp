#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/range.h>

#include <MPU9250.h>

// --- 핀 설정 ---
#define TRIG_L 5
#define ECHO_L 18
#define TRIG_C 19
#define ECHO_C 23
#define TRIG_R 32
#define ECHO_R 33

// --- 객체 ---
MPU9250 mpu(Wire, 0x68); // 주소 0x68 확인됨

rcl_publisher_t imu_pub;
rcl_publisher_t range_l_pub, range_c_pub, range_r_pub;

sensor_msgs__msg__Imu imu_msg;
sensor_msgs__msg__Range range_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

// --- 상태 변수 ---
int sonar_state = 0; // 0:Left, 1:Center, 2:Right

// --- 함수: 거리 측정 (Blocking 최소화) ---
float get_distance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  // 30ms 타임아웃 (약 5m)
  long duration = pulseIn(echo, HIGH, 30000); 
  if (duration == 0) return -1.0;
  
  // cm -> m 변환
  float dist_m = (duration * 0.034 / 2) / 100.0;
  if (dist_m < 0.2) return -1.0; // 최소 거리 미만 필터링
  return dist_m;
}

// --- 타이머 콜백 (Main Logic) ---
void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  (void) last_call_time;

  if (timer != NULL) {
    // 1. IMU 데이터 퍼블리싱 (매번 수행)
    if (mpu.readSensor()) {
        imu_msg.header.frame_id.data = (char*)"imu_link";
        
        // 시간 동기화는 Agent가 처리하지만, 필요시 여기서 millis() 기반 타임스탬프 추가 가능
        
        // 가속도 (m/s^2)
        imu_msg.linear_acceleration.x = mpu.getAccelX_mss();
        imu_msg.linear_acceleration.y = mpu.getAccelY_mss();
        imu_msg.linear_acceleration.z = mpu.getAccelZ_mss();
        
        // 자이로 (rad/s)
        imu_msg.angular_velocity.x = mpu.getGyroX_rads();
        imu_msg.angular_velocity.y = mpu.getGyroY_rads();
        imu_msg.angular_velocity.z = mpu.getGyroZ_rads();
        
        rcl_publish(&imu_pub, &imu_msg, NULL);
    }

    // 2. 초음파 데이터 퍼블리싱 (순차 수행)
    // 한 번 호출될 때마다 센서 하나만 처리하여 딜레이 분산
    float dist = 0.0;
    
    // Range 메시지 공통 설정
    range_msg.radiation_type = sensor_msgs__msg__Range__ULTRASOUND;
    range_msg.field_of_view = 0.52; // 30도
    range_msg.min_range = 0.2;
    range_msg.max_range = 4.0;

    switch (sonar_state) {
        case 0: // Left
            dist = get_distance(TRIG_L, ECHO_L);
            range_msg.header.frame_id.data = (char*)"sonar_left_link";
            range_msg.range = dist;
            rcl_publish(&range_l_pub, &range_msg, NULL);
            sonar_state = 1;
            break;
            
        case 1: // Center
            dist = get_distance(TRIG_C, ECHO_C);
            range_msg.header.frame_id.data = (char*)"sonar_center_link";
            range_msg.range = dist;
            rcl_publish(&range_c_pub, &range_msg, NULL);
            sonar_state = 2;
            break;
            
        case 2: // Right
            dist = get_distance(TRIG_R, ECHO_R);
            range_msg.header.frame_id.data = (char*)"sonar_right_link";
            range_msg.range = dist;
            rcl_publish(&range_r_pub, &range_msg, NULL);
            sonar_state = 0;
            break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  set_microros_serial_transports(Serial); // Serial 통신

  // 핀 설정
  pinMode(TRIG_L, OUTPUT); pinMode(ECHO_L, INPUT);
  pinMode(TRIG_C, OUTPUT); pinMode(ECHO_C, INPUT);
  pinMode(TRIG_R, OUTPUT); pinMode(ECHO_R, INPUT);

  // IMU 초기화
  Wire.begin();
  delay(500);
  mpu.begin();

  // Micro-ROS 설정
  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "miro_mcu_node", "", &support);

  // 퍼블리셔 등록
  rclc_publisher_init_default(&imu_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu), "imu/data");
  rclc_publisher_init_default(&range_l_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range), "ultrasonic/left");
  rclc_publisher_init_default(&range_c_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range), "ultrasonic/center");
  rclc_publisher_init_default(&range_r_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range), "ultrasonic/right");

  // 타이머 (20Hz = 50ms)
  // 50ms마다 콜백 실행 -> 초음파는 3번에 1번씩 도니까 150ms(약 6.6Hz) 주기로 갱신됨
  rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(50), timer_callback);
  
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_timer(&executor, &timer);
}

void loop() {
  // 데이터 송수신 처리
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
  delay(10);
}