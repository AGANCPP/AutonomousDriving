#include <stdint.h>
#include <vector>

using namespace std;
#ifndef BASEDEFINE_H
#define BASEDEFINE_H

#define NAVI_WINDOWNAME                     _T("MobileNavigator")

#define TEXT_BUFFER_MAX                     512
#define HALF_LANE_WIDTH                     1800
#define LANE_WIDTH                          3600

#define CAN_PERIOD                          100

#define THREAD_GPS_COURSEANGLE_ID           1000


#define WM_MESSAGE_CAMERAINFO               WM_USER+10001		// Camera 信息

#define WM_USER_CLOSEALL                    WM_USER+10002
#define WM_USER_LOG                         WM_USER+ 10003     // 



#define WM_CAM_START_MESSAGE                WM_USER + 10008    

#define WM_NAVI_MESSAGE                     WM_USER+10100     // 来自NAVI的消息
#define WM_LOG_PATH                         WM_USER + 10101      //  日志path
#define WM_START_LOG                        WM_USER + 10102      //  开启日志
#define WM_STOP_LOG                         WM_USER + 10103      //  关闭日志

#define WM_LANE_MESSAGE                     WM_USER + 10200   //来自LANE的消息

#define WM_MSGFROMFATHER                    WM_USER + 10301  // 从主窗口发来的消息
#define WM_MSGFROMFATHERCAN                 WM_USER + 10302  //  从主窗口发来的消息 从cameacan发来


#define WM_SHOWNAVICONTEXT                  WM_USER + 10104      //  显示Navi消息     
#define WM_SHOWLANECONTEXT                  WM_USER + 10105      //  显示Lane的消息

#define WM_SHOWVIDEODATA                    WM_USER + 10006      //  显示Video的消息


#define WM_STARTPROSWITCH                  WM_USER+19001   //启动关闭程序的开关   窗口间通信
#define WM_STARTLOGSWITCH                  WM_USER+19002   //启动关闭日志

#define WM_STARTVIDEO                      WM_USER+ 19003  //开启录像
#define WM_STOPVIDEO                       WM_USER+ 19004   //关闭录像
#define WM_STARTIMG                        WM_USER+ 19005   //开启图像
#define WM_STOPIMG                         WM_USER+ 19006   //关闭图像

#define WM_LOGMSG                          WM_USER + 20001   //日志消息
#define WM_VIDEOMSG                        WM_USER + 20002   //录像消息
#define WM_LOGPATHMSG                      WM_USER + 20003  // 日志路径消息
#define WM_IMGMSG                          WM_USER + 20004 // 图片开关消息

#define WM_SHOWNAVI                        WM_USER + 21001 // 显示navi
#define WM_SHOWLANE                        WM_USER + 21002 // 显示navi

#define WM_ENTERPARK                       WM_USER + 10201 // 进入停车场
#define WM_LEAVEPARK                       WM_USER + 10202 // 离开停车场

#define WM_TRAFFICSIGNAL_XYZ               WM_USER+10203      //  交通标识 X、Y
#define WM_LANEDISTANCE_LD_RD              WM_USER+10204      //  车线左右边线距离
#define WM_LANECHANGE_LEFT_RIGHT           WM_USER+10205      //  变道信息 左变道、右变道
#define WM_LANENUMBER                      WM_USER+10206      //  车线信息
#define WM_TRAFFICSIGNAL_TYPE              WM_USER+10207      //  交通标识

#define WM_CAMERA_TYPE                     WM_USER+10301      //  摄像头模式

#define WM_NAVI_VERSION                    WM_USER+10501 // 发送Navi版本信息
#define WM_CAMERACAN_VERSION               WM_USER+10502 // 发送CameraCAN版本信息

/*****20150309(begin)********/
#define WM_NAVI_CHANGELANE_SWITCHING       WM_USER+10503      //  向BootManager发送控制左右变道的开关
/*****20150309(end)**********/


#define WM_BOOTMANAGER_CHANGELANE          WM_USER+11001 //通过主控向Navi发送变道信息（鼠标点击处的车道号）

/*******20150306(begin)*********************************/
#define HK_NUMPAD1_ID                     20000        //按下1键左移一个车道
#define HK_NUMPAD2_ID                     20001        //按下2键左移一个车道
/*******20150306(end)***********************************/

typedef struct _stJPEGDATA
{
	int64_t    utime;
	int16_t    width;
	int16_t    height;
	int16_t    stride;
	int32_t    pixelformat;
	int32_t    size;
	std::vector< uint8_t > image;

	_stJPEGDATA()
	{
		utime = width = height = stride = pixelformat = size = 0;
	}

}stJPEGDATA;

 
typedef struct _stInMessage
{
	int MessageType;
	int MessageSwitch;

	_stInMessage()
	{
		MessageType = 0;
		MessageSwitch = 0;
	}
}stInMessage;


// Camera 信息
typedef struct _stCameraInfo
{
    DWORD  CameraCaptureTimestamp;          // Camera采集时间戳
    DWORD  CameraResultTimestamp;           // Camera处理结果时间戳
    int    LeftLane;                        // 左边车道数量
    int    RightLane;                       // 右边车道数量
    int    LDW;                             // 车线脱逸
    int    ChangeLaneEvent;                 // 变道事件
    int    LeftLineType;                    // 左边车线类型
    int    RightLineType;                   // 右边车线类型
    short  LeftLineDistance;                // 到左边车线的垂直距离 (单位:mm)
    short  RightLineDistance;               // 到右边车线的垂直距离 (单位:mm)
    short  SteeringAngle;                   // 航向角 (单位:角度)
    short  LateralDeviation;                // 横向偏移
	short  SignType;                        // traffic sign 类型
	short  SignPositionX;                   // traffic sign Position X
	short  SignPositionY;                   // traffic sign Position Y
	short  SignPositionZ;                   // traffic sign Position Z

    _stCameraInfo()
    {
        CameraCaptureTimestamp = CameraResultTimestamp = 0;
        LeftLane = RightLane = LDW = ChangeLaneEvent = LeftLineType = RightLineType = 0;
        LeftLineDistance = RightLineDistance = SteeringAngle = LateralDeviation = 0;
		SignType = SignPositionX = SignPositionY = SignPositionZ = 0;
        //Reserve = 0;
    }
}stCameraInfo;


#endif  // BASEDEFINE_H
