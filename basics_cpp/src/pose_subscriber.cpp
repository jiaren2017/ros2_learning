// pose_subscriber.cpp
// Subscribes to /turtle/status (Vector3) and logs the turtle's pose.

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/vector3.hpp>

class PoseSubscriber : public rclcpp::Node
{
public:
  PoseSubscriber() : Node("pose_subscriber")
  {
    subscriber_ = this->create_subscription<geometry_msgs::msg::Vector3>(
      "/turtle/status", 10,
      std::bind(&PoseSubscriber::status_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "PoseSubscriber node started.");
  }

private:
  void status_callback(const geometry_msgs::msg::Vector3 & msg)
  {
    RCLCPP_INFO(this->get_logger(),
      "Received pose → x: %.2f  y: %.2f  theta: %.2f",
      msg.x, msg.y, msg.z);
  }

  rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr subscriber_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PoseSubscriber>());
  rclcpp::shutdown();
  return 0;
}