
#ifndef IPC_ACCESS_H
#define IPC_ACCESS_H

#include "IpcMsgDef.h"

#ifdef DLL_IPC_ACCESS
    // dll export
    #define DLL_DEF extern "C" _declspec(dllexport)
#else
    #ifdef DLL_QT
        //dll import for qt
        #define DLL_DEF
    #else
        //dll import for vs
        #define DLL_DEF extern "C" _declspec(dllimport)
    #endif //DLL_QT
#endif  //DLL_IPC_ACCESS

#define IPC_WAIT_TIME (20000)

enum
{
    IPC_OK = 0,
    IPC_NG,
    IPC_OPERATER_FAIL,
};

// create ipc communication
DLL_DEF void CreateIpcComm(void);

// send image to module control
DLL_DEF void SendImageToDisplayQueue(int imageType_, int cols_, int rows_, int step_, int format_, int dataSize, unsigned char* pdata_);
// module control receive image from queue
DLL_DEF CAM_IMAGE_TYPE* GetFrontImageFromDisplayQueue(int imageType_);
DLL_DEF void PopImageFromDisplayQueue(int imageType_, CAM_IMAGE_TYPE* pImage_);

// module control receive notify form queue
DLL_DEF IPC_MSG_TYPE* GetFrontModuleControlNtyFromQueue(void);
DLL_DEF void PopModuleControlNtyFromQueue(IPC_MSG_TYPE* pMsg_);

// camera capture receive command from queue
DLL_DEF IPC_MSG_TYPE* GetFrontCamCmdFromQueue(void);
DLL_DEF void PopCamCmdFromQueue(IPC_MSG_TYPE* pMsg_);
DLL_DEF int SendCamRes(int msgId_, IPC_RES_DATA* pRes_);

// send command to camera capture
DLL_DEF int ReqCamStartDebug(void);
DLL_DEF int ReqCamEndWork(void);
DLL_DEF int ReqCamStartCapture(void);
DLL_DEF int ReqCamEndCapture(void);
DLL_DEF int ReqCamSetLogPath(char* path_);
DLL_DEF int ReqCamStartLog(void);
DLL_DEF int ReqCamEndLog(void);
DLL_DEF int ReqCamSetImgPath(char* path_);
DLL_DEF int ReqCamStartSaveImg(void);
DLL_DEF int ReqCamEndSaveImg(void);
DLL_DEF int ReqCamSetGrayValue(int grayValue_);
DLL_DEF int ReqCamSetExposureTime(int exposureTime_);
DLL_DEF int ReqCamEnableAutoExposure(int enable_);
DLL_DEF int ReqCamEnableCalibration(int enable_);
DLL_DEF int ReqCamEnableFull(int enable_);

// camera caputuer send notify to module control
DLL_DEF int NotifyCamProcessStart(void);
DLL_DEF int NotifyCamWorkStart(void);
DLL_DEF int NotifyCamIsFindingDevice(void);
DLL_DEF int NotifyCamFindedDeviceNum(int devNum_);
DLL_DEF int NotifyCamRecogType(int recogType_);
DLL_DEF int NotifyCamGrayValue(int grayValue_);
DLL_DEF int NotifyCamExposureTime(int exposureTime_);
DLL_DEF int NotifyCamVersion(char* version_);

// vehicle control receive command from queue
DLL_DEF IPC_MSG_TYPE* GetFrontVehCmdFromQueue(void);
DLL_DEF void PopVehCmdFromQueue(IPC_MSG_TYPE* pMsg_);
DLL_DEF int SendVehRes(int msgId_, IPC_RES_DATA* pRes_);

// send command to vehicle control
DLL_DEF int ReqVehFindWheelCtrlPort(void);
DLL_DEF int ReqVehOpenWheelCtrl(unsigned char add, const char* port);
DLL_DEF int ReqVehOpenCanCtrl(void);
DLL_DEF int ReqVehCloseWheelCtrl(void);
DLL_DEF int ReqVehCloseCanCtrl(void);
DLL_DEF int ReqVehStartCanRecv(void);
DLL_DEF int ReqVehEndWork(void);
// turn direction
enum
{
    WHEEL_TURN_RIGHT,
    WHEEL_TURN_LEFT,
};
// speed range 0 - 127
DLL_DEF int ReqVehTurnWheel(int direction_, int speed_);
// strength range 0x0000 - 0x0500
DLL_DEF int ReqVehBrake(int strength_);

// vehicle control send notify to module control
DLL_DEF int NotifyVehWorkStart(void);
DLL_DEF int NotifyVehEnumWheelCtrlPort(int all, int index, char* port);

#endif




