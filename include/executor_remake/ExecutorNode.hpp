#pragma once

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "executor_interfaces/action/execute_task.hpp"
#include "executor_tools/GoTo.hpp"
#include "executor_tools/Say.hpp"
#include "navigation_tools/Navigation.hpp"
#include "hri_tools/HRI.hpp"
#include "knowledge_tools/Knowledge.hpp"

#include <memory>

class ExecutorNode : public rclcpp::Node
{
public:
    using ExecuteTask = executor_interfaces::action::ExecuteTask;
    using GoalHandle  = rclcpp_action::ServerGoalHandle<ExecuteTask>;

    explicit ExecutorNode();

    // Must be called immediately after std::make_shared<ExecutorNode>(),
    // because shared_from_this() is only valid once the object is owned
    // by a shared_ptr.
    void init();

private:
    rclcpp_action::Server<ExecuteTask>::SharedPtr action_server_;

    std::shared_ptr<Navigation> navigation_;
    std::shared_ptr<HRI>        hri_;
    std::shared_ptr<Knowledge>  knowledge_;
    std::unique_ptr<GoTo>       goto_tool_;
    std::unique_ptr<Say>        say_tool_;

    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID& uuid,
        std::shared_ptr<const ExecuteTask::Goal> goal);

    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandle> goal_handle);

    void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle);

    // Runs in a detached thread — blocks until the tool finishes.
    void execute(const std::shared_ptr<GoalHandle> goal_handle);
};
