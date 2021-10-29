#include <ros/ros.h>
 #include <sensor_msgs/JointState.h>
 #include <xrobot_control/joint_states.h>
 #include <moveit/move_group_interface/move_group_interface.h>      

class TrajectoryController
{
    #define JOINTS_NUM  xrobot_control::joint_states::JOINTS_NUM
public:

    explicit TrajectoryController(std::string robot);

    /**
     * @brief 控制机械臂移动到目标点
     * @param target 目标点
     * @note 目标是各个关节的最终位置
     */
    void MoveToTarget(std::vector<double>& target);

    /**
     * @brief 控制机械臂移动到目标点
     * @param target 目标点
     * @note 目标是末端姿态
     */
    void MoveToTarget(geometry_msgs::Pose& target);

    /**
     * @brief 主控制函数
     * @param ctrl_Hz 控制频率
     */
    void Control(void)
    {
        // 开启单线程 
        ros::AsyncSpinner spinner(1);
        spinner.start();       

        // 控制机械臂回到起点
       _arm.setNamedTarget("home");
       _arm.move();
       sleep(1);

        ROS_INFO("Successfully init joints Position ");

        // 测试：A点
        double targetPose[7] = {0.391410, -0.676384, -0.376217, 0.0, 1.052834, 0.454125, 0.10111};
        std::vector<double> joint_group_positions(7);
        joint_group_positions[0] = targetPose[0];
        joint_group_positions[1] = targetPose[1];
        joint_group_positions[2] = targetPose[2];
        joint_group_positions[3] = targetPose[3];
        joint_group_positions[4] = targetPose[4];
        joint_group_positions[5] = targetPose[5];   
        joint_group_positions[6] = targetPose[6];   
        MoveToTarget(joint_group_positions);

        // 测试：B点
        geometry_msgs::Pose target_pose;
        target_pose.orientation.x = 1;
        target_pose.orientation.y = 0;
        target_pose.orientation.z = 0;
        target_pose.orientation.w = 0;

        target_pose.position.x = 0.2593;
        target_pose.position.y = 0.0636;
        target_pose.position.z = 0.1787;
        MoveToTarget(target_pose);

        // 关闭线程 
        ros::shutdown(); 
    }
private:

    // 机器人名称
    std::string  _robot;

    // 路径规划组
    moveit::planning_interface::MoveGroupInterface _arm;
};
