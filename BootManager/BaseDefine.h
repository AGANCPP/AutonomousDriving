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


#define WM_MESSAGE_CAMERAINFO               WM_USER+10001		// Camera ��Ϣ

#define WM_USER_CLOSEALL                    WM_USER+10002
#define WM_USER_LOG                         WM_USER+ 10003     // 



#define WM_CAM_START_MESSAGE                WM_USER + 10008    

#define WM_NAVI_MESSAGE                     WM_USER+10100     // ����NAVI����Ϣ
#define WM_LOG_PATH                         WM_USER + 10101      //  ��־path
#define WM_START_LOG                        WM_USER + 10102      //  ������־
#define WM_STOP_LOG                         WM_USER + 10103      //  �ر���־

#define WM_LANE_MESSAGE                     WM_USER + 10200   //����LANE����Ϣ

#define WM_MSGFROMFATHER                    WM_USER + 10301  // �������ڷ�������Ϣ
#define WM_MSGFROMFATHERCAN                 WM_USER + 10302  //  �������ڷ�������Ϣ ��cameacan����


#define WM_SHOWNAVICONTEXT                  WM_USER + 10104      //  ��ʾNavi��Ϣ     
#define WM_SHOWLANECONTEXT                  WM_USER + 10105      //  ��ʾLane����Ϣ

#define WM_SHOWVIDEODATA                    WM_USER + 10006      //  ��ʾVideo����Ϣ


#define WM_STARTPROSWITCH                  WM_USER+19001   //�����رճ���Ŀ���   ���ڼ�ͨ��
#define WM_STARTLOGSWITCH                  WM_USER+19002   //�����ر���־

#define WM_STARTVIDEO                      WM_USER+ 19003  //����¼��
#define WM_STOPVIDEO                       WM_USER+ 19004   //�ر�¼��
#define WM_STARTIMG                        WM_USER+ 19005   //����ͼ��
#define WM_STOPIMG                         WM_USER+ 19006   //�ر�ͼ��

#define WM_LOGMSG                          WM_USER + 20001   //��־��Ϣ
#define WM_VIDEOMSG                        WM_USER + 20002   //¼����Ϣ
#define WM_LOGPATHMSG                      WM_USER + 20003  // ��־·����Ϣ
#define WM_IMGMSG                          WM_USER + 20004 // ͼƬ������Ϣ

#define WM_SHOWNAVI                        WM_USER + 21001 // ��ʾnavi
#define WM_SHOWLANE                        WM_USER + 21002 // ��ʾnavi

#define WM_ENTERPARK                       WM_USER + 10201 // ����ͣ����
#define WM_LEAVEPARK                       WM_USER + 10202 // �뿪ͣ����

#define WM_TRAFFICSIGNAL_XYZ               WM_USER+10203      //  ��ͨ��ʶ X��Y
#define WM_LANEDISTANCE_LD_RD              WM_USER+10204      //  �������ұ��߾���
#define WM_LANECHANGE_LEFT_RIGHT           WM_USER+10205      //  �����Ϣ �������ұ��
#define WM_LANENUMBER                      WM_USER+10206      //  ������Ϣ
#define WM_TRAFFICSIGNAL_TYPE              WM_USER+10207      //  ��ͨ��ʶ

#define WM_CAMERA_TYPE                     WM_USER+10301      //  ����ͷģʽ

#define WM_NAVI_VERSION                    WM_USER+10501 // ����Navi�汾��Ϣ
#define WM_CAMERACAN_VERSION               WM_USER+10502 // ����CameraCAN�汾��Ϣ

/*****20150309(begin)********/
#define WM_NAVI_CHANGELANE_SWITCHING       WM_USER+10503      //  ��BootManager���Ϳ������ұ���Ŀ���
/*****20150309(end)**********/


#define WM_BOOTMANAGER_CHANGELANE          WM_USER+11001 //ͨ��������Navi���ͱ����Ϣ����������ĳ����ţ�

/*******20150306(begin)*********************************/
#define HK_NUMPAD1_ID                     20000        //����1������һ������
#define HK_NUMPAD2_ID                     20001        //����2������һ������
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


// Camera ��Ϣ
typedef struct _stCameraInfo
{
    DWORD  CameraCaptureTimestamp;          // Camera�ɼ�ʱ���
    DWORD  CameraResultTimestamp;           // Camera������ʱ���
    int    LeftLane;                        // ��߳�������
    int    RightLane;                       // �ұ߳�������
    int    LDW;                             // ��������
    int    ChangeLaneEvent;                 // ����¼�
    int    LeftLineType;                    // ��߳�������
    int    RightLineType;                   // �ұ߳�������
    short  LeftLineDistance;                // ����߳��ߵĴ�ֱ���� (��λ:mm)
    short  RightLineDistance;               // ���ұ߳��ߵĴ�ֱ���� (��λ:mm)
    short  SteeringAngle;                   // ����� (��λ:�Ƕ�)
    short  LateralDeviation;                // ����ƫ��
	short  SignType;                        // traffic sign ����
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
