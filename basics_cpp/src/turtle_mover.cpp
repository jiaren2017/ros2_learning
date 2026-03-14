// turtle_mover.cpp
// Timer-driven Twist publisher with runtime-configurable Parameters.

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>

class TurtleMover : public rclcpp::Node
{
public:
  TurtleMover() : Node("turtle_mover")
  {
    // Declare parameters with default values
    // These can be overridden at launch or changed at runtime
    this->declare_parameter<double>("linear_speed", 1.0);
    this->declare_parameter<double>("angular_speed", 1.0);

    // Read initial values into member variables
    linear_speed_  = this->get_parameter("linear_speed").as_double();
    angular_speed_ = this->get_parameter("angular_speed").as_double();

    // Register a callback that fires whenever any parameter is changed externally
    // e.g. via: ros2 param set /turtle_mover linear_speed 2.0
    param_callback_handle_ = this->add_on_set_parameters_callback(
      std::bind(&TurtleMover::on_parameter_change, this, std::placeholders::_1));

    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>(
      "/turtle1/cmd_vel", 10);

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(1000),
      std::bind(&TurtleMover::timer_callback, this));

    RCLCPP_INFO(this->get_logger(),
      "TurtleMover started. linear=%.1f angular=%.1f",
      linear_speed_, angular_speed_);
  }

private:
  // Called automatically when someone does: ros2 param set /turtle_mover <name> <value>
  // Must return a SetParametersResult indicating accept or reject
  rcl_interfaces::msg::SetParametersResult on_parameter_change(
    const std::vector<rclcpp::Parameter> & parameters)
  {
    for (const auto & param : parameters) {
      if (param.get_name() == "linear_speed") {
        linear_speed_ = param.as_double();
        RCLCPP_INFO(this->get_logger(), "linear_speed updated: %.1f", linear_speed_);
      } else if (param.get_name() == "angular_speed") {
        angular_speed_ = param.as_double();
        RCLCPP_INFO(this->get_logger(), "angular_speed updated: %.1f", angular_speed_);
      }
    }

    rcl_interfaces::msg::SetParametersResult result;
    result.successful = true;
    return result;
  }

  void timer_callback()
  {
    auto msg = geometry_msgs::msg::Twist();
    msg.linear.x  = linear_speed_;
    msg.angular.z = angular_speed_;
    publisher_->publish(msg);
  }

  double linear_speed_;
  double angular_speed_;

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;


  // Handle must be stored as member — if it goes out of scope, timer callback is unregistered!
  rclcpp::TimerBase::SharedPtr timer_;

  // Handle must be stored as member — if it goes out of scope, param callback is unregistered!
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TurtleMover>());
  rclcpp::shutdown();
  return 0;
}