// turtle_arc_client.cpp
// Action Client: sends a rotation goal to TurtleArcServer,
// monitors feedback, and reports the final result.

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <my_interfaces/action/rotate_turtle.hpp>

class TurtleArcClient : public rclcpp::Node
{
public:
  using RotateTurtle = my_interfaces::action::RotateTurtle;
  using GoalHandle   = rclcpp_action::ClientGoalHandle<RotateTurtle>;

  TurtleArcClient() : Node("turtle_arc_client")
  {
    client_ = rclcpp_action::create_client<RotateTurtle>(this, "rotate_turtle");
    RCLCPP_INFO(this->get_logger(), "TurtleArcClient initialized.");
  }

  // Returns false if server not available or goal was rejected
  bool run(float target_angle)
  {
    // Wait for action server
    RCLCPP_INFO(this->get_logger(), "Waiting for action server...");
    while (!client_->wait_for_action_server(std::chrono::seconds(1))) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(this->get_logger(), "Interrupted. Exiting.");
        return false;
      }
      RCLCPP_INFO(this->get_logger(), "Still waiting...");
    }

    // Build the goal
    auto goal = RotateTurtle::Goal();
    goal.target_angle = target_angle;

    // SendGoalOptions bundles the three callbacks together
    auto options = rclcpp_action::Client<RotateTurtle>::SendGoalOptions();

    // Called when server accepts or rejects the goal
    options.goal_response_callback =
      std::bind(&TurtleArcClient::goal_response_callback, this,
        std::placeholders::_1);

    // Called every time server publishes feedback
    options.feedback_callback =
      std::bind(&TurtleArcClient::feedback_callback, this,
        std::placeholders::_1, std::placeholders::_2);

    // Called when the goal reaches a terminal state: SUCCEEDED / CANCELED / ABORTED
    options.result_callback =
      std::bind(&TurtleArcClient::result_callback, this,
        std::placeholders::_1);

    RCLCPP_INFO(this->get_logger(),
      "Sending goal: target_angle=%.2f rad", target_angle);

    client_->async_send_goal(goal, options);
    return true;
  }

private:
  void goal_response_callback(const GoalHandle::SharedPtr & goal_handle)
  {
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "Goal rejected by server.");
      rclcpp::shutdown();
      return;
    }
    RCLCPP_INFO(this->get_logger(), "Goal accepted by server.");
  }

  void feedback_callback(
    GoalHandle::SharedPtr,
    const std::shared_ptr<const RotateTurtle::Feedback> feedback)
  {
    RCLCPP_INFO(this->get_logger(),
      "Feedback: remaining=%.3f rad", feedback->remaining_angle);
  }

  void result_callback(const GoalHandle::WrappedResult & result)
  {
    switch (result.code) {
      case rclcpp_action::ResultCode::SUCCEEDED:
        RCLCPP_INFO(this->get_logger(),
          "Result: SUCCEEDED. Total rotation=%.3f rad",
          result.result->total_rotation);
        break;
      case rclcpp_action::ResultCode::CANCELED:
        RCLCPP_WARN(this->get_logger(), "Result: CANCELED.");
        break;
      case rclcpp_action::ResultCode::ABORTED:
        RCLCPP_ERROR(this->get_logger(), "Result: ABORTED.");
        break;
      default:
        RCLCPP_ERROR(this->get_logger(), "Result: UNKNOWN.");
        break;
    }
    rclcpp::shutdown();
  }

  rclcpp_action::Client<RotateTurtle>::SharedPtr client_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<TurtleArcClient>();

  // Accept target_angle from command line, default to 3.14 rad
  float target = 3.14f;
  if (argc > 1) {
    target = std::stof(argv[1]);
  }

  if (node->run(target)) {
    rclcpp::spin(node);
  }

  rclcpp::shutdown();
  return 0;
}