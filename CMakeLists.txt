cmake_minimum_required(VERSION 3.8)
project(executor_remake)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(executor_interfaces REQUIRED)
find_package(executor_tools REQUIRED)
find_package(hri_tools REQUIRED)
find_package(navigation_tools REQUIRED)
find_package(knowledge_tools REQUIRED)

add_executable(executor_node
  src/ExecutorNode.cpp
  src/executor_main.cpp
)

target_include_directories(executor_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(executor_node
  rclcpp
  rclcpp_action
  executor_interfaces
  executor_tools
  hri_tools
  navigation_tools
  knowledge_tools
)

install(TARGETS executor_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
