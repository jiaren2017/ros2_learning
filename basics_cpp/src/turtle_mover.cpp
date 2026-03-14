// turtle_mover.cpp
// Uses a Timer to publish Twist commands at 1Hz, making turtle draw circles.

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>

class TurtleMover : public rclcpp::Node
{
public:
  TurtleMover() : Node("turtle_mover")
  {
    // Publisher: send velocity commands to turtlesim
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>(
      "/turtle1/cmd_vel", 10);

    // Timer: call timer_callback() every 1000ms (1Hz)
    // create_timer(interval, callback)
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(1000),
      std::bind(&TurtleMover::timer_callback, this));

    RCLCPP_INFO(this->get_logger(), "TurtleMover node started. Publishing at 1Hz.");
  }

private:
  void timer_callback()
  {
    auto msg = geometry_msgs::msg::Twist();

    // linear.x: move forward at 1.0 m/s
    // angular.z: rotate at 1.0 rad/s
    // Both non-zero → turtle draws a circle
    msg.linear.x = 1.0;
    msg.angular.z = 1.0;

    publisher_->publish(msg);

    RCLCPP_DEBUG(this->get_logger(), "Published Twist: linear=%.1f angular=%.1f",
      msg.linear.x, msg.angular.z);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TurtleMover>());
  rclcpp::shutdown();
  return 0;
}