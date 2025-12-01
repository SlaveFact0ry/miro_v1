#!/bin/bash
set -e

# 1. ROS2 Humble 환경 변수 로드
source "/opt/ros/humble/setup.bash"

# 2. Micro-ROS Agent 빌드 환경 로드
if [ -f "/microros_ws/install/setup.bash" ]; then
    source "/microros_ws/install/setup.bash"
fi

# 3. 워크스페이스(Miro System) 빌드 환경 로드
if [ -f "/root/ros2_ws/install/setup.bash" ]; then
    source "/root/ros2_ws/install/setup.bash"
fi

# 4. Docker CMD 명령어 실행
exec "$@"
