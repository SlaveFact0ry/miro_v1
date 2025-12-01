FROM ros:humble-ros-base

# 1. 기본 툴 및 Udev/USB 관련 패키지 설치
# python3-colcon-common-extensions: 빌드 툴
# usbutils: lsusb 등 디버깅용
RUN apt-get update && apt-get install -y \
    python3-pip \
    python3-colcon-common-extensions \
    git \
    udev \
    usbutils \
    ros-humble-rmw-cyclonedds-cpp \
    && rm -rf /var/lib/apt/lists/*

# 2. Micro-ROS Agent 빌드 (시간 소요 구간)
WORKDIR /microros_ws
RUN git clone -b humble https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup
# rosdep update 및 의존성 설치 추가 (에러 방지)
RUN . /opt/ros/humble/setup.sh && \
    apt-get update && \
    rosdep init || echo "rosdep already initialized" && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src -y && \
    colcon build && \
    . install/local_setup.sh && \
    ros2 run micro_ros_setup create_agent_ws.sh && \
    ros2 run micro_ros_setup build_agent.sh

# 3. 사용자 워크스페이스 복사
WORKDIR /root/ros2_ws
# 소스 코드가 바뀔 때마다 캐시가 깨지도록 순서 배치
COPY ./src /root/ros2_ws/src

# 4. 의존성 설치 및 빌드 (Miro System)
RUN . /opt/ros/humble/setup.sh && \
    apt-get update && \
    rosdep install --from-paths src --ignore-src -r -y && \
    colcon build --symlink-install

# 5. Entrypoint 설정
COPY ./entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
