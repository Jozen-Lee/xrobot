 #include <ros/ros.h>
#include <actionlib/server/simple_action_server.h>
#include <control_msgs/FollowJointTrajectoryAction.h>
#include <sensor_msgs/JointState.h>
#include <xrobot_control/joint_states.h>
 #include <serial/serial.h>

 class JointsStatesController
 {
    #define JOINTS_NUM  xrobot_control::joint_states::JOINTS_NUM
    typedef actionlib::SimpleActionServer<control_msgs::FollowJointTrajectoryAction> Action_Server; 
 public:
    JointsStatesController(std::string robot, std::string port_name, std::int32_t baudrate = 115200, std::int16_t out_time = 100) 
    : _robot(robot),
    _as(_node, robot + "/arm_joint_controller/follow_joint_trajectory",  
                        boost::bind(&JointsStatesController::Revtrajectory_CB, this, _1),  false)
    {
        // 串口初始化
        serial::Timeout serial_timeout = serial::Timeout::simpleTimeout(out_time); 
        serial_port.setPort(port_name);
        serial_port.setBaudrate(baudrate);
        serial_port.setTimeout(serial_timeout);

        // 节点初始化
        Node_Init();

        // 关节数据初始化
        _js.name.resize(JOINTS_NUM);
        _js.position.resize(JOINTS_NUM);   // 分配内存，不然会报错
        _js.header.stamp = ros::Time::now();
        for(int i = 0; i < JOINTS_NUM; i ++) 
        {
            _js.name[i] = joints_name[i];
            _js.position[i] = 0;
        }
    }
    ~JointsStatesController(){};

    /**
     * @brief 接收轨迹点
     * @param goal move_group计算所得的目标点
     */
    void Revtrajectory_CB(const control_msgs::FollowJointTrajectoryGoalConstPtr& goal);

    /**
     * @brief 关节信息发布函数
     */    
    void JointsStatesPublish(void)
    {
        _joints_states_puber.publish(_js);
    }

    /**
     * @brief 打开串口，循环收发数据，与下位机进行通信
     * @param ctrl_Hz 读取串口数据的频率
     */      
    void start(void);
 private:

    // 串口
    serial::Serial serial_port;

    // 关节信息订阅者及发布者
    ros::NodeHandle _node;
    ros::Publisher _joints_states_puber;
    ros::Publisher _wb_joints_states_puber;
    ros::Subscriber _wb_joints_states_suber;

    // 串口数据存储区
    static const uint16_t REV_LEN = JOINTS_NUM*4 +1;
    static const uint16_t TRAN_LEN = JOINTS_NUM*4 +1;
    std::uint8_t serial_revbuf[REV_LEN];
    std::uint8_t serial_transbuf[TRAN_LEN];

    // 机器人名字
    std::string _robot;

    // 动作服务端
    Action_Server _as;

    // 关节状态
    sensor_msgs::JointState _js;

    // 轨迹点存储变量
    std::vector<trajectory_msgs::JointTrajectoryPoint> _points;

    // 关节名称
    std::string joints_name[JOINTS_NUM] = {"joint1", "joint2", "joint3", "joint4", "joint5", "joint6", "joint7"};

     /**
     * @brief 开启串口
     */    
    void Open_Serial(void)
    {
        try
        {    
            serial_port.open();
        }
        catch(serial::IOException& e)
        {
            ROS_ERROR("Unable to open the serial port");
        }
        if(serial_port.isOpen()) ROS_INFO("Successfully open the serial port");
    }

    /**
     * @brief 节点初始化
     */    
    void Node_Init(void)
    {
        // 创建关节信息发布者，发布给轨迹控制器
        _joints_states_puber = _node.advertise<sensor_msgs::JointState>("/"+ _robot + "/joint_states", 10);

        // 创建关节信息发布者，发布给Webots
        _wb_joints_states_puber = _node.advertise<sensor_msgs::JointState>("/webots/"+ _robot + "/joint_states", 10);
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
     * @brief 发送数据给下位机
     */     
    void WriteToDsp(std::vector<double>& msg)
    {
        // 串口打开再发送
        if(serial_port.isOpen())
        {
            // 打包数据
            PackJointsStates(serial_transbuf, msg);

            // // 串口发送
            serial_port.write(serial_transbuf, JOINTS_NUM*4 + 1);
        }
    }

    /**
     * @brief 更新关节状态，true 成功更新 false 无更新
     * @note 其中包括了下位机通信，使用前需要确保串口开启
     */     
    bool JointsStatesUpdate(void)
    {
        std::vector<float> js(JOINTS_NUM);

        if(serial_port.isOpen())
        {
            // 读取串口中有效数据个数
            size_t recv_size = serial_port.available();
            if(recv_size > 0)
            {
                // 读取串口数据
                recv_size = serial_port.read(serial_revbuf, recv_size);

                // 数据解包
                if(UnpackJointsStates(js, serial_revbuf))
                {
                    Update_Joints_States(js);
                    return true;
                }
                else return false;
            }
            else return false;
        }
        else return false;
    }

    /**
     * @brief  发送电机数据给webots
     */     
    void SendDatatoWebots(sensor_msgs::JointState& msg)
    {
       _wb_joints_states_puber.publish(msg);
    }

    /**
     * @brief 关节状态解包函数
     */     
    template<typename  _Ty>
    bool UnpackJointsStates(std::vector<_Ty>& dst, const uint8_t* src)
    {
        int data;
        // 校验位判断
        if(src[JOINTS_NUM*4] == JOINTS_NUM) 
        {
            ROS_INFO("Successfully Receive Joints States");

            // 数据转存
            for(int i = 0; i < JOINTS_NUM; i++) 
            {
                memcpy(&data, &src[i*4], 4);
                dst[i] = data / 1000;
            }
                return true;
        }       
        else return false;
    }

    /**
     * @brief 关节状态打包函数
     */     
    template<typename  _Ty>
    void PackJointsStates(uint8_t* dst, const std::vector<_Ty>& src)
    {
        int data;

        // 确保关节数目正确
        assert(src.size() == JOINTS_NUM);

        // 数据转存
        for(int i = 0; i < JOINTS_NUM; i++)
        {
            data = src[i] * 1000;
            memcpy(&dst[i*4], &data, 4);
        } 

        // 校验位
        serial_transbuf[JOINTS_NUM*4] = JOINTS_NUM;
    }
};
