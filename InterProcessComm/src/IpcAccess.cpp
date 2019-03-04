
#include "stdafx.h"
#include <algorithm>

#ifndef DLL_IPC_ACCESS
    #define DLL_IPC_ACCESS
#endif

#include "IpcAccess.h"
#include "ComProcess.h"

typedef c_map_type<int, IPC_RES_DATA> ipc_res_type;

// camera image queue
static ComInterProcessMsgQueue<CAM_IMAGE_TYPE, 2> m_signImageQueue(true, TEXT("CAMERA_SIGN_IMAGE_DISPLAY_QUEUE"));
static ComInterProcessMsgQueue<CAM_IMAGE_TYPE, 2> m_laneImageQueue(true, TEXT("CAMERA_LANE_IMAGE_DISPLAY_QUEUE"));
static ComInterProcessMsgQueue<CAM_IMAGE_TYPE, 2> m_roadImageQueue(true, TEXT("CAMERA_ROAD_IMAGE_DISPLAY_QUEUE"));
// camera capture command queue
static ComInterProcessMsgQueue<IPC_MSG_TYPE, 10, ipc_res_type> m_cameraCommandQueue(true, TEXT("CAMERA_ACCESS_COMMAND_QUEUE"));
// module control receive notify form this queue
static ComInterProcessMsgQueue<IPC_MSG_TYPE, 10> m_moduleControlNotifyQueue(true, TEXT("MODULE_CONTROL_NOTIFY_QUEUE"));
// vehicle control command queue
static ComInterProcessMsgQueue<IPC_MSG_TYPE, 10, ipc_res_type> m_vehicleControlCommandQueue(true, TEXT("VEHICLE_CONTROL_COMMAND_QUEUE"));

bool s_bInitFlag = false;

// create ipc communication
void CreateIpcComm(void)
{
    if (false == s_bInitFlag)
    {
        m_signImageQueue.Create();
        m_laneImageQueue.Create();
        m_roadImageQueue.Create();
        m_cameraCommandQueue.Create(true);
        m_moduleControlNotifyQueue.Create();
        m_vehicleControlCommandQueue.Create(true);
        s_bInitFlag = true;
    }
}

// send image to module control
void SendImageToDisplayQueue(int imageType_, int cols_, int rows_, int step_, int format_, int dataSize_, unsigned char* pdata_)
{
    //COM_DEBUG(DEBUG_LEVEL3, ("IPC: SendImageToDisplayQueue, imageType_(%d) cols_(%d) rows_(%d) dataSize_(%u)\n",
    //  imageType_, cols_, rows_, dataSize_));
    if ((dataSize_ < 1) || (NULL == pdata_))
    {
        COM_ERR("IPC: SendImageToDisplayQueue, invalid parameter\n");
        return;
    }
    CAM_IMAGE_TYPE* pimage = NULL;
    if (CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN == imageType_)
        pimage = m_signImageQueue.AllocSpace();
    else if (CAM_IMAGE_TYPE::IMAGE_TYPE_LANE == imageType_)
        pimage = m_laneImageQueue.AllocSpace();
    else if (CAM_IMAGE_TYPE::IMAGE_TYPE_ROAD == imageType_)
        pimage = m_roadImageQueue.AllocSpace();
    else
        COM_ERR("[ERR] IPC: SendImageToDisplayQueue, invalid imageType_(%d)\n", imageType_);
    if (NULL != pimage)
    {
        pimage->image_type = imageType_;
        pimage->cols = cols_;
        pimage->rows = rows_;
        pimage->step = step_;
        pimage->format = format_;
        pimage->imgSize = dataSize_;
        memcpy(pimage->data, pdata_, std::min<int>(dataSize_, CAM_IMAGE_TYPE::IMAGE_SIZE));
        if (CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN == imageType_)
            m_signImageQueue.PushBack(pimage);
        else if (CAM_IMAGE_TYPE::IMAGE_TYPE_LANE == imageType_)
            m_laneImageQueue.PushBack(pimage);
        else if (CAM_IMAGE_TYPE::IMAGE_TYPE_ROAD == imageType_)
            m_roadImageQueue.PushBack(pimage);
    }
    else
        COM_ERR("[ERR] IPC: SendImageToDisplayQueue, image queue.AllocSpace err\n");
}

//module control receive image from queue
CAM_IMAGE_TYPE* GetFrontImageFromDisplayQueue(int imageType_)
{
    CAM_IMAGE_TYPE* pimage = NULL;
    if (CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN == imageType_)
        pimage = m_signImageQueue.GetFront();
    else if (CAM_IMAGE_TYPE::IMAGE_TYPE_LANE == imageType_)
        pimage = m_laneImageQueue.GetFront();
    else if (CAM_IMAGE_TYPE::IMAGE_TYPE_ROAD == imageType_)
        pimage = m_roadImageQueue.GetFront();
    else
        COM_ERR("[ERR] IPC: GetFrontImageFromDisplayQueue, invalid imageType_(%d)\n", imageType_);
    return (pimage);
}

void PopImageFromDisplayQueue(int imageType_, CAM_IMAGE_TYPE* pImage_)
{
    if (NULL == pImage_)
        return;
    if (CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN == imageType_)
        m_signImageQueue.FreeSpace(pImage_);
    else if (CAM_IMAGE_TYPE::IMAGE_TYPE_LANE == imageType_)
        m_laneImageQueue.FreeSpace(pImage_);
    else if (CAM_IMAGE_TYPE::IMAGE_TYPE_ROAD == imageType_)
        m_roadImageQueue.FreeSpace(pImage_);
    else
        COM_ERR("[ERR] IPC: PopImageFromDisplayQueue, invalid imageType_(%d)\n", imageType_);
}


//module control receive notify form queue
IPC_MSG_TYPE* GetFrontModuleControlNtyFromQueue(void)
{
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.GetFront();
    return (pMsg);
}

void PopModuleControlNtyFromQueue(IPC_MSG_TYPE* pMsg_)
{
    if (NULL != pMsg_)
        m_moduleControlNotifyQueue.FreeSpace(pMsg_);
}


//camera capture receive command from queue
IPC_MSG_TYPE* GetFrontCamCmdFromQueue(void)
{
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.GetFront();
    return (pMsg);
}

void PopCamCmdFromQueue(IPC_MSG_TYPE* pMsg_)
{
    if (NULL != pMsg_)
        m_cameraCommandQueue.FreeSpace(pMsg_);
}

int SendCamRes(int msgId_, IPC_RES_DATA* pRes_)
{
    ipc_res_type* res = m_cameraCommandQueue.AllocResSpace(IPC_WAIT_TIME);
    if (NULL == res)
    {
        COM_ERR("[ERR] IPC: %s, AllocResSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    res->getKey() = msgId_;
    if (NULL != pRes_)
        res->getData() = *pRes_;
    m_cameraCommandQueue.SendRes(res);
    return (IPC_OK);
}

//send command to camera capture
int ReqCamStartDebug(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_START_DEBUG;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_START_DEBUG, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR] IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamEndWork(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_END_WORK;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_END_WORK, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR] IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamStartCapture(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_START_CAPTURE;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_START_CAPTURE, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamEndCapture(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_END_CAPTURE;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_END_CAPTURE, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR] IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamSetLogPath(char* path_)
{
    int nRet = IPC_NG;
    if (NULL == path_)
        return (nRet);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC %s, path: %s\n", __FUNCTION__, path_));
    int pathLen = strlen(path_);
    if (pathLen > IPC_MSG_TYPE::MAX_CHAR_SIZE - 1)
    {
        COM_ERR("[ERR] IPC: %s, path lenth too long\n", __FUNCTION__);
        return (nRet);
    }
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_SET_LOG_PATH;
        strncpy(pMsg->msg.logPath, path_, pathLen);
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_SET_LOG_PATH, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR] IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamStartLog(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_START_LOG;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_START_LOG, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR] IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamEndLog(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_END_LOG;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_END_LOG, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamSetImgPath(char* path_)
{
    int nRet = IPC_NG;
    if (NULL == path_)
        return (nRet);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, path: %s\n", __FUNCTION__, path_));
    int pathLen = strlen(path_);
    if (pathLen > IPC_MSG_TYPE::MAX_CHAR_SIZE - 1)
    {
        COM_ERR("[ERR]IPC: %s, path lenth too long\n", __FUNCTION__);
        return (nRet);
    }
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_SET_IMG_PATH;
        strncpy(pMsg->msg.imgPath, path_, pathLen);
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_SET_IMG_PATH, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamStartSaveImg(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_START_SAVE_IMG;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_START_SAVE_IMG, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamEndSaveImg(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_END_SAVE_IMG;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_END_SAVE_IMG, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}


int ReqCamSetGrayValue(int grayValue_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_SET_GRAY_VALUE;
        pMsg->msg.cam.grayValue = grayValue_;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_SET_GRAY_VALUE, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqCamSetExposureTime(int exposureTime_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_SET_EXPOSURE_TIME;
        pMsg->msg.cam.exposureTime = exposureTime_;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_SET_EXPOSURE_TIME, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}


int ReqCamEnableAutoExposure(int enable_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_ENABLE_AUTO_EXPOSURE;
        pMsg->msg.cam.enableAutoExposure = enable_;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_ENABLE_AUTO_EXPOSURE, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}


int ReqCamEnableCalibration(int enable_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_cameraCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_CMD_ENABLE_CALIBRATION;
        pMsg->msg.cam.enableCalibration = enable_;
        m_cameraCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_cameraCommandQueue.WaitRes(MSG_CAM_CMD_ENABLE_CALIBRATION, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_cameraCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

/*
    camera caputuer send notify to module control
*/
int NotifyCamProcessStart(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_PROCESS_START;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

int NotifyCamWorkStart(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_WORK_START;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

int NotifyCamIsFindingDevice(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_IS_FINDING_DEVICE;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

int NotifyCamFindedDeviceNum(int devNum_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_FINDED_DEVICE_NUM;
        pMsg->msg.cameraNum = devNum_;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}


int NotifyCamRecogType(int recogType_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_RECOG_TYPE;
        pMsg->msg.cam.recogType = recogType_;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

int NotifyCamGrayValue(int grayValue_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_GRAY_VALUE;
        pMsg->msg.cam.grayValue = grayValue_;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

int NotifyCamExposureTime(int exposureTime_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_EXPOSURE_TIME;
        pMsg->msg.cam.exposureTime = exposureTime_;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

int NotifyCamVersion(char* version_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_CAM_NTY_VERSION;
        strncpy(pMsg->msg.version, version_, IPC_MSG_TYPE::MAX_CHAR_SIZE - 1);
        pMsg->msg.version[IPC_MSG_TYPE::MAX_CHAR_SIZE - 1] = 0;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}


/*
    vehicle control receive command from queue (m_vehicleControlCommandQueue)
*/
IPC_MSG_TYPE* GetFrontVehCmdFromQueue(void)
{
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.GetFront();
    return (pMsg);
}

void PopVehCmdFromQueue(IPC_MSG_TYPE* pMsg_)
{
    if (NULL != pMsg_)
        m_vehicleControlCommandQueue.FreeSpace(pMsg_);
}

int SendVehRes(int msgId_, IPC_RES_DATA* pRes_)
{
    ipc_res_type* res = m_vehicleControlCommandQueue.AllocResSpace(IPC_WAIT_TIME);
    if (NULL == res)
    {
        COM_ERR("[ERR] IPC: %s, AllocResSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    res->getKey() = msgId_;
    if (NULL != pRes_)
        res->getData() = *pRes_;
    m_vehicleControlCommandQueue.SendRes(res);
    return (IPC_OK);
}

/*
    send command to vehicle control
*/

int ReqVehFindWheelCtrlPort(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_FIND_WHEEL_CTRL_PORT;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]CAM: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_FIND_WHEEL_CTRL_PORT, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]CAM: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehOpenWheelCtrl(unsigned char add, const char* port)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    if (NULL == port)
    {
        COM_ERR("[ERR] IPC: %s, port is null\n", __FUNCTION__);
        return (IPC_NG);
    }
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_OPEN_WHEEL_CTRL;
        pMsg->msg.wheel.port.addr = add;
        strncpy(pMsg->msg.wheel.port.port, port, 15);
        pMsg->msg.wheel.port.port[15] = 0;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]CAM: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_OPEN_WHEEL_CTRL, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]CAM: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehOpenCanCtrl(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_OPEN_CAN_CTRL;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]CAM: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_OPEN_CAN_CTRL, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]CAM: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehCloseWheelCtrl(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_CLOSE_WHEEL_CTRL;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]CAM: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_CLOSE_WHEEL_CTRL, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]CAM: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehCloseCanCtrl(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_CLOSE_CAN_CTRL;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]CAM: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_CLOSE_CAN_CTRL, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]CAM: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehStartCanRecv(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_START_CAN_RECV;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]CAM: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_START_CAN_RECV, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]CAM: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehEndWork(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_END_WORK;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]CAM: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_END_WORK, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]CAM: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehTurnWheel(int direction_, int speed_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s (%d, %d)\n", __FUNCTION__, direction_, speed_));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_TURN_WHEEL;
        pMsg->msg.wheel.direction = direction_;
        pMsg->msg.wheel.speed = speed_;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR]IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_TURN_WHEEL, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR]IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

int ReqVehBrake(int strength_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s (%d)\n", __FUNCTION__, strength_));
    int nRet = IPC_NG;
    IPC_MSG_TYPE* pMsg = m_vehicleControlCommandQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_CMD_BRAKE;
        pMsg->msg.brake.strength = strength_;
        m_vehicleControlCommandQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (nRet);
    }
    ipc_res_type* pRes = m_vehicleControlCommandQueue.WaitRes(MSG_VEH_CMD_BRAKE, IPC_WAIT_TIME);
    if (NULL == pRes)
    {
        COM_ERR("[ERR] IPC: %s, WaitRes err\n", __FUNCTION__);
        return (nRet);
    }
    nRet = pRes->getData().result;
    m_vehicleControlCommandQueue.FreeResSpace(pRes);
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s, WaitRes ok (ret=%d)\n", __FUNCTION__, nRet));
    return (nRet);
}

/*
    vehicle control send notify to module control
*/
int NotifyVehWorkStart(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s\n", __FUNCTION__));
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_NTY_WORK_START;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

int NotifyVehEnumWheelCtrlPort(int all, int index, char* port)
{
    COM_DEBUG(DEBUG_LEVEL3, ("IPC: %s (%d, %d, %s)\n", __FUNCTION__, all, index, port));
    if (NULL == port)
    {
        COM_ERR("[ERR] IPC: %s, port is null\n", __FUNCTION__);
        return (IPC_NG);
    }
    IPC_MSG_TYPE* pMsg = m_moduleControlNotifyQueue.AllocSpace(IPC_WAIT_TIME);
    if (NULL != pMsg)
    {
        pMsg->msgId = MSG_VEH_NTY_ENUM_WHEEL_CTRL_PORT;
        pMsg->msg.wheel.enum_port.all = all;
        pMsg->msg.wheel.enum_port.index = index;
        strncpy(pMsg->msg.wheel.enum_port.port, port, 15);
        pMsg->msg.wheel.enum_port.port[15] = 0;
        m_moduleControlNotifyQueue.PushBack(pMsg);
    }
    else
    {
        COM_ERR("[ERR] IPC: %s, AllocSpace err\n", __FUNCTION__);
        return (IPC_NG);
    }
    return (IPC_OK);
}

