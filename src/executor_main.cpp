#include "executor_remake/ExecutorNode.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ExecutorNode>();
    node->init();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
