#include <xrobot_control/xrobot_trajectory_controller.h>

/**
 * @brief 构造函数，初始化了ROS的节点功能
 */
TrajectoryController::TrajectoryController(std::string robot) :
 _robot(robot), _arm("manipulator")
{}

/**
 * @brief 控制机械臂移动到目标点
 * @param target 目标点
 * @note 目标是各个关节的最终位置
 */
void TrajectoryController::MoveToTarget(std::vector<double>& target)
{
    // 保证关节点的数量符合要求
    assert(target.size() == JOINTS_NUM);

    // 设置路径规划参数
    _arm.setGoalJointTolerance(0.001);                           // 关节允许误差 
    _arm.setMaxAccelerationScalingFactor(0.2);          // 允许的最大加速度
    _arm.setMaxVelocityScalingFactor(0.2);                   // 允许的最大速度

    // 机械臂执行
    ROS_INFO("Start Moving");
    _arm.setJointValueTarget(target);
    _arm.move();
    sleep(1);
    ROS_INFO("Moving Finish");
}

/**
 * @brief 控制机械臂移动到目标点
 * @param target 目标点
 * @note 目标是末端姿态
 */
void TrajectoryController::MoveToTarget(geometry_msgs::Pose& target)
{
    // 获取终端link的名称 
    std::string end_effector_link = _arm.getEndEffectorLink();

    // 设置目标位置所使用的参考坐标系 
    std::string reference_frame = "base_link";
    _arm.setPoseReferenceFrame(reference_frame);    

    // 当运动规划失败后，允许重新规划 
    _arm.allowReplanning(true);

    // 设置路径规划的参数  
    _arm.setGoalPositionTolerance(0.001); 
    _arm.setGoalOrientationTolerance(0.01);
    _arm.setMaxAccelerationScalingFactor(0.2);
    _arm.setMaxVelocityScalingFactor(0.2);

    // 设置机器臂当前的状态作为运动初始状态
    _arm.setStartStateToCurrentState();

    // 设置目标状态 
    _arm.setPoseTarget(target);

    // 计算机器人移动到目标的运动轨迹 
    moveit::planning_interface::MoveGroupInterface::Plan plan;
    moveit::planning_interface::MoveItErrorCode success = _arm.plan(plan);
    ROS_INFO("Plan (pose goal) %s",success?"":"FAILED");   

    // 控制机械臂按既定轨迹运动
    if(success) _arm.execute(plan);
    sleep(1);
}

int main(int argc, char* argv[])
{
    //ROS节点初始化
    ros::init(argc, argv, "trajectory_controller");

    //创建控制实体
    TrajectoryController controller("xrobot");

    //开始运行move_group
    controller.Control();
    return 0;
}


