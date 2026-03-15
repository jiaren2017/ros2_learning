// lifecycle_turtle_mover.cpp
// A lifecycle-managed node that drives turtlesim in circles.
// Transitions: configure → activate (start timer) → deactivate (stop timer) → cleanup

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <geometry_msgs/msg/twist.hpp>

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class LifecycleTurtleMover : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit LifecycleTurtleMover(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : LifecycleNode("lifecycle_turtle_mover", options)
  {
    // Declare parameter here so it's visible before configure()
    this->declare_parameter<double>("linear_speed", 1.0);
    this->declare_parameter<double>("angular_speed", 0.8);
    RCLCPP_INFO(get_logger(), "Node constructed. Waiting for configure().");
  }

  // --- Transition Callbacks ---

  CallbackReturn on_configure(const rclcpp_lifecycle::State &)
  {
    // Allocate publisher — but do NOT activate it yet
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);

    linear_speed_  = this->get_parameter("linear_speed").as_double();
    angular_speed_ = this->get_parameter("angular_speed").as_double();

    RCLCPP_INFO(get_logger(), "Configured. linear=%.2f angular=%.2f", linear_speed_, angular_speed_);
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State & state)
  {
    // Must call parent to activate the publisher's internal lifecycle
    LifecycleNode::on_activate(state);

    // Start the timer — turtle begins moving
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),
      std::bind(&LifecycleTurtleMover::timer_callback, this));

    RCLCPP_INFO(get_logger(), "Activated. Turtle is moving.");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state)
  {
    // Must call parent to deactivate the publisher
    LifecycleNode::on_deactivate(state);

    // Cancel timer — turtle stops, but publisher still exists
    timer_->cancel();
    timer_.reset();

    RCLCPP_INFO(get_logger(), "Deactivated. Turtle stopped.");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &)
  {
    // Release all resources — back to Unconfigured
    timer_.reset();
    publisher_.reset();

    RCLCPP_INFO(get_logger(), "Cleaned up. Resources released.");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &)
  {
    // Final teardown — called from any state on shutdown
    timer_.reset();
    publisher_.reset();

    RCLCPP_INFO(get_logger(), "Shutdown complete.");
    return CallbackReturn::SUCCESS;
  }

private:
  void timer_callback()
  {
    auto msg = geometry_msgs::msg::Twist();
    msg.linear.x  = linear_speed_;
    msg.angular.z = angular_speed_;
    publisher_->publish(msg);
  }

  // Lifecycle-aware publisher — only publishes when node is Active
  rclcpp_lifecycle::LifecyclePublisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  double linear_speed_{1.0};
  double angular_speed_{0.8};
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  // LifecycleNode is not a rclcpp::Node, must extract the base interface explicitly
  auto node = std::make_shared<LifecycleTurtleMover>();
  rclcpp::spin(node->get_node_base_interface());
  
  rclcpp::shutdown();
  return 0;
}