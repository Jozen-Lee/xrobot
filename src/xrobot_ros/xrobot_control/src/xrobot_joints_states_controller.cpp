#include <xrobot_control/xrobot_joints_states_controller.h>

/**
 * @brief 接收轨迹点
 * @param goal move_group计算所得的轨迹点
 */
void JointsStatesController::Revtrajectory_CB(const control_msgs::FollowJointTrajectoryGoalConstPtr& goal)
{
    // 发布成功接收信息 
   ROS_INFO("Receive New Goal");

    // 读取轨迹点
    _points = goal->trajectory.points;

    // 按照一定频率发布
    uint16_t points_len = _points.size(); uint16_t index = 0;
    ros::Rate loop_rate(20);
    while(ros::ok() && index < points_len)
    {
        // 发送数据给下位机
        WriteToDsp(_points.at(index).positions);

        // 反馈关节信息给move_group，用于测试
        for (int i = 0; i < JOINTS_NUM; i++)  _js.position[i] = _points.at(index).positions[i];
        JointsStatesPublish();
        
        // 发送数据给webots
        SendDatatoWebots(_js);

        // 查找下一个轨迹点
        index ++;

        // 查看中断
        ros::spinOnce();

        // 延时
        loop_rate.sleep();
    }

    // 告诉action客户端成功信息
    control_msgs::FollowJointTrajectoryResult result;
    result.error_code = 0;
    _as.setSucceeded(result);
}

/**
 * @brief 打开串口，循环收发数据，与下位机进行通信
 */    
void JointsStatesController::start(void)
{
    // 打开串口
    Open_Serial();

    // 开启服务器
    _as.start();

    // 测试
    std::vector<double> msg(JOINTS_NUM);
    std::vector<float> js(JOINTS_NUM);
    for (int i = 0; i < JOINTS_NUM; i++)
    {
        msg[i] = i;
    }

    ros::Rate loop_rate(20);
    // 循环收发串口数据
    while (ros::ok())
    {
            // 发送到下位机
            WriteToDsp(msg);

            // 更新关节状态
            if(JointsStatesUpdate()) 
            {
                // 发布关节状态信息
                JointsStatesPublish();
            }

        // 延时等待
        loop_rate.sleep();

        // 查看中断函数
        ros::spinOnce();
    }

    // 程序关闭
    serial_port.close();
    ros::waitForShutdown();
}
int main(int argc, char* argv[])
{
    // 初始化节点
    ros::init(argc, argv, "joints_states_node");

    // 创建控制器
    JointsStatesController ctrl_node("xrobot", "/dev/TuTu_USB1");

    // 开启串口，进行数据收发
    ctrl_node.start();
    return 0;
} 
 
