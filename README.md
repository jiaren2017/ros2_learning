# ROS 2 Learning Journey

This repository documents my progress in learning **ROS 2 (Jazzy Jalisco)** on **Ubuntu 24.04 LTS**.

## 🛠 Development Environment
- **CPU:** AMD Ryzen 7 9800X3D (8C/16T)
- **OS:** Ubuntu 24.04 LTS
- **ROS 2 Version:** Jazzy Jalisco
- **Middleware:** Cyclone DDS
- **IDE:** Visual Studio Code

## 📂 Repository Structure
- `my_first_node/`: A basic Python-based ROS 2 package for testing environment setup.

## 🚀 How to build
Navigate to your workspace root:
```bash
cd ~/ros2_ws
colcon build --symlink-install --parallel-workers 16
source install/setup.bash