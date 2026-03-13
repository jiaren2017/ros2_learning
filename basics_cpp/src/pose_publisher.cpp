// pose_publisher.cpp
// Subscribes to /turtle1/pose from turtlesim,
// then republishes (x, y, theta) to /turtle/status as Vector3.

#include <rclcpp/rclcpp.hpp>
#include <turtlesim/msg/pose.hpp>
#include <geometry_msgs/msg/vector3.hpp>

class PosePublisher : public rclcpp::Node
{
public:
  PosePublisher() : Node("pose_publisher")
  {
    // Publisher: broadcast turtle position to /turtle/status
    // topic: /turtle/status
    // QoS queue depth: 10
    publisher_ = this->create_publisher<geometry_msgs::msg::Vector3>(
      "/turtle/status", 10);

    // Subscriber: listen to turtlesim's native pose topic
    // alternative with lambda funciton: [this](const turtlesim::msg::Pose & msg) { pose_callback(msg); }
    // each time a new pose is received, call pose_callback()
    subscriber_ = this->create_subscription<turtlesim::msg::Pose>(
      "/turtle1/pose", 10,
      std::bind(&PosePublisher::pose_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "PosePublisher node started.");
  }

private:
  void pose_callback(const turtlesim::msg::Pose & msg)
  {
    // Pack (x, y, theta) into a Vector3 message
    auto out = geometry_msgs::msg::Vector3();
    out.x = msg.x;
    out.y = msg.y;
    out.z = msg.theta;  // z carries orientation angle

    publisher_->publish(out);

    RCLCPP_DEBUG(this->get_logger(),
      "Republished pose: x=%.2f  y=%.2f  theta=%.2f",
      out.x, out.y, out.z);
  }

  rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr publisher_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr subscriber_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PosePublisher>());
  rclcpp::shutdown();
  return 0;
}