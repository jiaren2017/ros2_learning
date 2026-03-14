// turtle_reset_client.cpp
// One-shot service client: calls /reset to return turtle to origin.
// Pattern: constructor initializes only, run() owns lifecycle logic, main() is minimal.

#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/empty.hpp>

class TurtleResetClient : public rclcpp::Node
{
public:
  TurtleResetClient() : Node("turtle_reset_client")
  {
    // create_client<ServiceType>("service_name")
    // ServiceType: must match exactly what the server registered (std_srvs::srv::Empty)
    // "/reset": the service name turtlesim registered on the DDS network
    client_ = this->create_client<std_srvs::srv::Empty>("/reset");

    RCLCPP_INFO(this->get_logger(), "TurtleResetClient node initialized.");
  }

  // run() owns the full lifecycle: wait → send → return control to main()
  // Returns true if request was sent successfully, false if interrupted
  bool run()
  {
    RCLCPP_INFO(this->get_logger(), "Waiting for /reset service...");

    while (!client_->service_is_ready()) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(this->get_logger(),
          "Interrupted while waiting for /reset service. Exiting.");
        return false;
      }
      rclcpp::sleep_for(std::chrono::seconds(1));
      RCLCPP_INFO(this->get_logger(), "Still waiting for /reset service...");
    }

    send_reset_request();
    return true;
  }

private:
  void send_reset_request()
  {
    // Empty request: no fields to fill, just needs to exist as a valid object
    auto request = std::make_shared<std_srvs::srv::Empty::Request>();

    // async_send_request: non-blocking
    // ROS2 will call reset_response_callback() when turtlesim replies
    client_->async_send_request(
      request,
      std::bind(&TurtleResetClient::reset_response_callback, this,
        std::placeholders::_1));
  }

  // Triggered by ROS2 executor when turtlesim sends back the response
  void reset_response_callback(
    rclcpp::Client<std_srvs::srv::Empty>::SharedFuture future)
  {
    // Response is Empty — receiving it at all means success
    (void)future;
    RCLCPP_INFO(this->get_logger(),
      "Reset successful. Turtle returned to origin.");

    // One-shot node: task complete, trigger clean shutdown
    rclcpp::shutdown();
  }

  rclcpp::Client<std_srvs::srv::Empty>::SharedPtr client_;
};

// main() is minimal: init → construct → run → spin → shutdown
int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<TurtleResetClient>();

  // run() handles waiting and sending — spin() only needed if request was sent
  if (node->run()) {

    // spin required to wait for future to resolve
    rclcpp::spin(node); 
  }

  rclcpp::shutdown();
  return 0;
}