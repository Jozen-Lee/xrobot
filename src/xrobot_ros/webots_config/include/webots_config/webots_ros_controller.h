 #include <ros/ros.h>
#include <actionlib/server/simple_action_server.h>
#include <control_msgs/FollowJointTrajectoryAction.h>
#include <sensor_msgs/JointState.h>
#include <xrobot_control/joint_states.h>

 class WebotsController
 {
    #define JOINTS_NUM  xrobot_control::joint_states::JOINTS_NUM
    typedef actionlib::SimpleActionServer<control_msgs::FollowJointTrajectoryAction> Action_Server; 
 public:
    WebotsController(std::string robot) 
    : _robot(robot)
    {
        // 节点初始化
        Node_Init();

    }
    ~WebotsController(){};


    /**
     * @brief 打开串口，循环收发数据，与下位机进行通信
     * @param ctrl_Hz 读取串口数据的频率
     */      
    void start(void);
 private:

    // 关节信息订阅者及发布者
    ros::NodeHandle _node;
    ros::Publisher _joints_states_puber;              // 发布给move_group
    ros::Publisher _wb_joints_states_puber;     // 发布给webots
    ros::Subscriber _trajectory_suber;

    // 机器人名字
    std::string _robot;

    // 动作服务端
    Action_Server _as;

    // 关节状态
    sensor_msgs::JointState _js;

    // 轨迹点存储变量
    std::vector<trajectory_msgs::JointTrajectoryPoint> _points;

    // 关节名称
    std::string joints_name[JOINTS_NUM] = {"Joint1", "Joint2", "Joint3", "Joint4", "Joint5", "Joint6", "Joint7"};

    /**
     * @brief 节点初始化
     */    
    void Node_Init(void)
    {
        // 创建关节信息发布者，发布给轨迹控制器
        _joints_states_puber = _node.advertise<sensor_msgs::JointState>("/"+ _robot + "/joint_states", 10);


    }

    /**
     * @brief 更新关节状态
     */     
    template<typename  _Ty>
    void Update_Joints_States(const std::vector<_Ty>& msg)
    {
        // 更新时间
        _js.header.stamp = ros::Time::now();

        // 更新位置
        for(int i = 0; i < JOINTS_NUM; i++) _js.position[i] = msg[i];
    }

    /**
     * @brief 发送数据给webots
     */     
    void SendToWebots(std::vector<double>& msg)
    {

    }

};
