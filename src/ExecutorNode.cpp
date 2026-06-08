#include "executor_remake/ExecutorNode.hpp"
#include "executor_interfaces/msg/task_result.hpp"
#include <thread>

using namespace std::placeholders;

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / init
// ─────────────────────────────────────────────────────────────────────────────

ExecutorNode::ExecutorNode()
: Node("executor_node")
{}

void ExecutorNode::init()
{
    auto self = shared_from_this();

    navigation_ = std::make_shared<Navigation>(self);
    hri_        = std::make_shared<HRI>(self);
    knowledge_  = std::make_shared<Knowledge>(self);
    goto_tool_  = std::make_unique<GoTo>(self, navigation_);
    say_tool_   = std::make_unique<Say>(self, hri_, knowledge_);

    action_server_ = rclcpp_action::create_server<ExecuteTask>(
        this,
        "execute_task",
        std::bind(&ExecutorNode::handle_goal,     this, _1, _2),
        std::bind(&ExecutorNode::handle_cancel,   this, _1),
        std::bind(&ExecutorNode::handle_accepted, this, _1)
    );

    RCLCPP_INFO(get_logger(), "ExecutorNode ready — action server: 'execute_task'.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Action server callbacks
// ─────────────────────────────────────────────────────────────────────────────

rclcpp_action::GoalResponse ExecutorNode::handle_goal(
    const rclcpp_action::GoalUUID&,
    std::shared_ptr<const ExecuteTask::Goal> goal)
{
    RCLCPP_INFO(get_logger(),
        "Received step [%u]: action='%s' args[0]='%s'",
        goal->task.step_id,
        goal->task.action.c_str(),
        goal->task.args.empty() ? "" : goal->task.args[0].c_str());

    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse ExecutorNode::handle_cancel(
    const std::shared_ptr<GoalHandle>)
{
    RCLCPP_WARN(get_logger(), "Cancel request received.");
    return rclcpp_action::CancelResponse::ACCEPT;
}

void ExecutorNode::handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
{
    // Spawn a thread so the action server can accept new goals immediately.
    std::thread{std::bind(&ExecutorNode::execute, this, goal_handle)}.detach();
}

// ─────────────────────────────────────────────────────────────────────────────
// Execution (runs in a detached thread)
// ─────────────────────────────────────────────────────────────────────────────

void ExecutorNode::execute(const std::shared_ptr<GoalHandle> goal_handle)
{
    const auto& task   = goal_handle->get_goal()->task;
    const auto& action = task.action;
    const auto& args   = task.args;

    Result tool_result;

    if (action == "goto") {
        tool_result = args.empty()
            ? Result{false, "FAILED", "goto: missing location argument"}
            : goto_tool_->run(args[0], goal_handle);

    } else if (action == "say") {
        tool_result = args.empty()
            ? Result{false, "FAILED", "say: missing phrase argument"}
            : say_tool_->run(args[0], goal_handle);

    } else {
        tool_result = {false, "FAILED", "unknown_action: " + action};
        RCLCPP_ERROR(get_logger(), "Unknown action: '%s'", action.c_str());
    }

    if (!goal_handle->is_active()) {
        RCLCPP_WARN(get_logger(), "Goal was cancelled during step [%u].", task.step_id);
        return;
    }

    auto result = std::make_shared<ExecuteTask::Result>();
    result->result.step_id = task.step_id;
    result->result.status  = tool_result.success
        ? executor_interfaces::msg::TaskResult::SUCCESS
        : executor_interfaces::msg::TaskResult::FAILURE;
    result->result.message = tool_result.message;

    if (tool_result.success) {
        goal_handle->succeed(result);
        RCLCPP_INFO(get_logger(),
            "Step [%u] SUCCEEDED: %s", task.step_id, tool_result.message.c_str());
    } else {
        goal_handle->abort(result);
        RCLCPP_WARN(get_logger(),
            "Step [%u] FAILED: %s", task.step_id, tool_result.message.c_str());
    }
}
