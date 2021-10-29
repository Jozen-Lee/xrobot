// include 
//#include <webots_config/webots_ros_controller.h>

// int controllerCount = 0;
// std::vector<std_msgs::String> controllerList;
// std::string
// /*============================================== Callback ================================================*/
//     void controllerNameCallback(const std_msgs::String::ConstPtr &name) 
//     { 
//         controllerCount++; 
//         controllerList.push_back(name->data);
//         //将控制器名加入到列表中 
//         ROS_INFO("Controller #%d: %s.", controllerCount, controllerList.back().c_str());
// }
/*============================================== Client =============================================*/
/*============================================== Subscriber =============================================*/

#include "ros/ros.h"

#include <webots_ros/Int32Stamped.h>

#include <webots_ros/set_float.h>
#include <webots_ros/set_int.h>

#include <webots_ros/robot_get_device_list.h>

#include <std_msgs/String.h>
#include <sensor_msgs/JointState.h>
#include <stdio.h>

#define TIME_STEP 32

static int controllerCount;
static std::vector<std::string> controllerList;
static std::string controllerName;
static double lposition = 0;
static double rposition = 0;
std::string robot_name = "xrobot";

// 定义电机驱动的客户端
ros::ServiceClient Joint1_Client;
webots_ros::set_float Joint1_Srv;
ros::ServiceClient Joint2_Client;
webots_ros::set_float Joint2_Srv;
ros::ServiceClient Joint3_Client;
webots_ros::set_float Joint3_Srv;
ros::ServiceClient Joint4_Client;
webots_ros::set_float Joint4_Srv;
ros::ServiceClient Joint5_Client;
webots_ros::set_float Joint5_Srv;
ros::ServiceClient Joint6_Client;
webots_ros::set_float Joint6_Srv;
ros::ServiceClient Joint7_Client;
webots_ros::set_float Joint7_Srv;

// 时间戳同步
ros::ServiceClient timeStepClient;
webots_ros::set_int timeStepSrv;

// 关节状态发布者及订阅者
ros::Subscriber wb_joints_status_suber; 
ros::Publisher wb_joints_status_puber;

// catch names of the controllers availables on ROS network
void controllerNameCallback(const std_msgs::String::ConstPtr &name)
{
  controllerCount++;
  controllerList.push_back(name->data);
  ROS_INFO("Controller #%d: %s.", controllerCount, controllerList.back().c_str());
}

void JointsStatusCallback(const sensor_msgs::JointState& msg)
{
    Joint1_Srv.request.value = msg.position[0];  Joint1_Client.call(Joint1_Srv);
    Joint2_Srv.request.value = msg.position[1];  Joint2_Client.call(Joint2_Srv);
    Joint3_Srv.request.value = msg.position[2];  Joint3_Client.call(Joint3_Srv);
    Joint4_Srv.request.value = msg.position[3];  Joint4_Client.call(Joint4_Srv);
    Joint5_Srv.request.value = msg.position[4];  Joint5_Client.call(Joint5_Srv);
    Joint6_Srv.request.value = msg.position[5];  Joint6_Client.call(Joint6_Srv);
    Joint7_Srv.request.value = msg.position[6];  Joint7_Client.call(Joint7_Srv);
}


int main(int argc, char **argv) 
{
    // 节点初始化
    ros::init(argc, argv, "webots_ros_ctrl", ros::init_options::AnonymousName);
    ros::NodeHandle n;

    // 查看可用的控制器
    ros::Subscriber nameSub = n.subscribe("model_name", 100, controllerNameCallback);
    while (controllerCount == 0 || controllerCount < nameSub.getNumPublishers()) 
    {
        ros::spinOnce();
        ros::Duration(0.5).sleep();
    }
    ros::spinOnce();

    // 多于一个控制器的情况就让用户进行选择
    if (controllerCount == 1)
    controllerName = controllerList[0];
    else 
    {
        int wantedController = 0;
        std::cout << "Choose the # of the controller you want to use:\n";
        std::cin >> wantedController;
        if (1 <= wantedController && wantedController <= controllerCount)
            controllerName = controllerList[wantedController - 1];
        else
        {
            ROS_ERROR("Invalid number for controller choice.");
            return 1;
        }
    }
    // 删去已经无效的订阅者
    nameSub.shutdown();

    // 时间同步客户端
    timeStepClient = n.serviceClient<webots_ros::set_int>(controllerName + "/robot/time_step");
    timeStepSrv.request.value = TIME_STEP;

    // 关节状态客户端定义
    Joint1_Client = n.serviceClient<webots_ros::set_float>("/"+robot_name+ "/joint1" + "/set_position");
    Joint2_Client = n.serviceClient<webots_ros::set_float>("/"+robot_name+ "/joint2" + "/set_position");
    Joint3_Client = n.serviceClient<webots_ros::set_float>("/"+robot_name+ "/joint3" + "/set_position");
    Joint4_Client = n.serviceClient<webots_ros::set_float>("/"+robot_name+ "/joint4" + "/set_position");
    Joint5_Client = n.serviceClient<webots_ros::set_float>("/"+robot_name+ "/joint5" + "/set_position");
    Joint6_Client = n.serviceClient<webots_ros::set_float>("/"+robot_name+ "/joint6" + "/set_position");
    Joint7_Client = n.serviceClient<webots_ros::set_float>("/"+robot_name+ "/joint7" + "/set_position");
    
    // 订阅关节信息
    wb_joints_status_suber =  n.subscribe("/webots/"+robot_name+"/joint_states", 100, JointsStatusCallback);

    // 循环等待
    while (ros::ok())
    {
        ros::spinOnce();
        if (!timeStepClient.call(timeStepSrv) || !timeStepSrv.response.success)
        ROS_ERROR("Failed to call service time_step for next step.");
    }
  
    // 关闭节点
    timeStepSrv.request.value = 0;
    timeStepClient.call(timeStepSrv);
    ros::shutdown();
    return 0;
}

