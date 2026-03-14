// turtle_arc_server.cpp
// Action Server: receives a target_angle goal, rotates turtle by that amount,
// publishes feedback (remaining_angle) every 0.5s, returns total_rotation as result.

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <turtlesim/msg/pose.hpp>
#include <my_interfaces/action/rotate_turtle.hpp>

#include <cmath>

class TurtleArcServer : public rclcpp::Node
{
public:
  // Type alias for readability — avoids repeating the full template type
  using RotateTurtle = my_interfaces::action::RotateTurtle;
  using GoalHandle   = rclcpp_action::ServerGoalHandle<RotateTurtle>;

  TurtleArcServer() : Node("turtle_arc_server")
  {
    // Publisher: sends velocity commands to move the turtle
    cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
      "/turtle1/cmd_vel", 10);

    // Subscriber: tracks current turtle orientation (theta)
    pose_sub_ = this->create_subscription<turtlesim::msg::Pose>(
      "/turtle1/pose", 10,
      std::bind(&TurtleArcServer::pose_callback, this, std::placeholders::_1));

    // Action server: registers three callbacks
    // handle_goal     → called when a new goal arrives, decide to accept or reject
    // handle_cancel   → called when client requests cancellation
    // handle_accepted → called after goal is accepted, starts execution
    action_server_ = rclcpp_action::create_server<RotateTurtle>(
      this,
      "rotate_turtle",
      std::bind(&TurtleArcServer::handle_goal,     this, std::placeholders::_1, std::placeholders::_2),
      std::bind(&TurtleArcServer::handle_cancel,   this, std::placeholders::_1),
      std::bind(&TurtleArcServer::handle_accepted, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "TurtleArcServer ready.");
  }

private:
  // --- Pose callback: keeps current_theta_ up to date ---
  void pose_callback(const turtlesim::msg::Pose & msg)
  {
    if (first_pose_) {
      last_theta_ = msg.theta;
      first_pose_ = false;
      current_theta_ = msg.theta;
      return;
    }

    // Compute the shortest angular delta, handles ±π wraparound correctly
    float delta = msg.theta - last_theta_;

    // Normalize delta to [-π, π]
    if (delta >  M_PI) delta -= 2.0f * M_PI;
    if (delta < -M_PI) delta += 2.0f * M_PI;

    total_rotated_ += std::abs(delta);
    last_theta_     = msg.theta;
    current_theta_  = msg.theta;
  }

  // --- Goal callback: decide whether to accept or reject incoming goal ---
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const RotateTurtle::Goal> goal)
  {
    (void)uuid;
    RCLCPP_INFO(this->get_logger(),
      "Received goal: target_angle=%.2f rad", goal->target_angle);

    // Reject goals with zero or negative angle
    if (goal->target_angle <= 0.0f) {
      RCLCPP_WARN(this->get_logger(), "Rejected: target_angle must be positive.");
      return rclcpp_action::GoalResponse::REJECT;
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  // --- Cancel callback: decide whether to honour a cancellation request ---
  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandle> goal_handle)
  {
    (void)goal_handle;
    RCLCPP_INFO(this->get_logger(), "Cancel request received. Accepting.");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  // --- Accepted callback: goal was accepted, spin up execution in a new thread ---
  // Action execution must NOT block the ROS2 executor thread.
  // Running in a detached thread keeps spin() free to process callbacks.
  void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    std::thread{
      std::bind(&TurtleArcServer::execute, this, std::placeholders::_1),
      goal_handle
    }.detach();
  }

  // --- Execute: runs in a separate thread, performs the rotation ---
void execute(const std::shared_ptr<GoalHandle> goal_handle)
{
  const float target = goal_handle->get_goal()->target_angle;

  // Reset accumulator at the start of each goal
  total_rotated_ = 0.0f;

  auto feedback = std::make_shared<RotateTurtle::Feedback>();
  auto cmd      = geometry_msgs::msg::Twist();

  RCLCPP_INFO(this->get_logger(), "Executing: rotating %.2f rad", target);

  rclcpp::Rate rate(20); // 20Hz loop，每 50ms 检查一次

  while (rclcpp::ok()) {
    if (goal_handle->is_canceling()) {
      auto result          = std::make_shared<RotateTurtle::Result>();
      result->total_rotation = total_rotated_;
      goal_handle->canceled(result);
      cmd.angular.z = 0.0;
      cmd_pub_->publish(cmd);
      RCLCPP_INFO(this->get_logger(), "Goal canceled. Rotated: %.2f rad", total_rotated_);
      return;
    }

    if (total_rotated_ >= target) {
      break;
    }

    float remaining = target - total_rotated_;

    // Three-zone speed control:
    // far zone    (remaining > 0.5 rad) → full speed  1.0 rad/s
    // near zone   (remaining > 0.1 rad) → slow speed  0.2 rad/s
    // final zone  (remaining ≤ 0.1 rad) → creep speed 0.05 rad/s
    if (remaining > 0.5f) {
      cmd.angular.z = 1.0f;
    } else if (remaining > 0.1f) {
      cmd.angular.z = 0.2f;
    } else {
      cmd.angular.z = 0.05f;
    }

    feedback->remaining_angle = remaining;
    goal_handle->publish_feedback(feedback);
    RCLCPP_INFO(this->get_logger(), "Feedback: remaining=%.2f rad", feedback->remaining_angle);

    cmd_pub_->publish(cmd);
    rate.sleep();
  }

  // Stop the turtle
  cmd.angular.z = 0.0;
  cmd_pub_->publish(cmd);

  auto result          = std::make_shared<RotateTurtle::Result>();
  result->total_rotation = total_rotated_;
  goal_handle->succeed(result);
  RCLCPP_INFO(this->get_logger(),
    "Goal succeeded. Target: %.2f rad  Actual: %.2f rad  Error: %.3f rad",
    target, total_rotated_, total_rotated_ - target);
}

  // Callback for pose updates
  float current_theta_{0.0f};
  float last_theta_{0.0f};
  float total_rotated_{0.0f};
  bool  first_pose_{true};


  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr        cmd_pub_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr          pose_sub_;
  rclcpp_action::Server<RotateTurtle>::SharedPtr                 action_server_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TurtleArcServer>());
  rclcpp::shutdown();
  return 0;
}