// camControl.cpp

/******************************************************************************
 * Head files
 ******************************************************************************/
#include "camControl.h"
#include "commondebug.h"

/******************************************************************************
 * class
 ******************************************************************************/
//#define CAM_CAPTURE_EXE_NAME "..\\CamCapture_single\\Release\\CamCapture.exe"
//#define CAM_CAPTURE_EXE_NAME "D:\\Project\\autonomous_driving\\K2W\\code\\AutonomousDrivingControl\\CamCapture_single\\Release\\CamCapture.exe"
#define CAM_CAPTURE_EXE_NAME "D:\\Project\\autonomous_driving\\K2W\\code\\AutonomousDrivingControl\\CamCapture_double\\Release\\CamCapture.exe"
CamControl::CamControl()
{
    m_nCamStatus = CAM_STATUS_IDLE;
    lstrcpy(m_szCamCaptureProcess, TEXT(CAM_CAPTURE_EXE_NAME));
    memset(&m_siCamCaptureProcess, 0, sizeof(m_siCamCaptureProcess));
    memset(&m_piCamCaptureProcess, 0, sizeof(m_piCamCaptureProcess));
    m_siCamCaptureProcess.cb = sizeof(m_siCamCaptureProcess);
    m_piCamCaptureProcess.hProcess = NULL;

    CreateIpcComm();
    m_hThreadQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_bThreadRunFlag = false;
}

CamControl::~CamControl()
{
    if (m_bThreadRunFlag) {
        EndCam();
    }
    CloseHandle(m_hThreadQuitEvent);
}

int CamControl::GetCamStatus(void)
{
    return (m_nCamStatus);
}

void CamControl::ChangeCamStatus(int status_)
{
    COM_DEBUG(3, ("Change cam status from (%d) to (%d)\n", m_nCamStatus, status_));
    m_nCamStatus = status_;
}

bool CamControl::StartCam(void)
{
    COM_DEBUG(3, ("Func : %s\n", __FUNCTION__));

    BOOL bRetValue = FALSE;

    int camStatus = GetCamStatus();
    if (CAM_STATUS_IDLE != camStatus) {
        COM_WARN("[WARN] Cam status error (%d)\n", camStatus);
        return (false);
    }

    ChangeCamStatus(CAM_STATUS_STARTING);
    // create CamCapture process
    bRetValue = CreateProcess(NULL, m_szCamCaptureProcess, NULL, NULL, FALSE, 0, NULL, NULL, &m_siCamCaptureProcess, &m_piCamCaptureProcess);
    if (FALSE == bRetValue) {
        ChangeCamStatus(CAM_STATUS_IDLE);
        COM_ERR("Create CamCapture Process Error!\n");
        return (false);
    }

    m_bThreadRunFlag = true;
    this->start();

    COM_DEBUG(3, ("Func : %s, create CamCapture process ok\n", __FUNCTION__));

    return (true);
}

bool CamControl::EndCam(void)
{
    COM_DEBUG(3, ("Func : %s\n", __FUNCTION__));

    DWORD dwRetValue = 0;

    int camStatus = GetCamStatus();
    if (CAM_STATUS_RUNNING != camStatus) {
        COM_WARN("[WARN]CamControl::EndCam, Cam status error (%d)\n", camStatus);
        return (false);
    }

    ChangeCamStatus(CAM_STATUS_ENDING);

    COM_DEBUG(3, ("Send camera end work message to CamCapture...\n"));
    ReqCamEndWork();

    COM_DEBUG(3, ("Waiting CamCapture process exit...\n"));
    dwRetValue = WaitForSingleObject(m_piCamCaptureProcess.hProcess, 20000);
    switch (dwRetValue) {
    case WAIT_OBJECT_0:
        break;
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
        COM_ERR("Waiting CamCapture process exit...error\n");
    default:
        break;
    };

    ChangeCamStatus(CAM_STATUS_IDLE);

    COM_DEBUG(3, ("Waiting CamCapture process exit...ok\n"));

    // stop thread
    m_bThreadRunFlag = false;
    SetEvent(m_hThreadQuitEvent);
    bool bRet = wait(5000);
    if (false == bRet) {
        COM_ERR("[ERR] Stop cam message receiving thread error");
        return (false);
    }

    emit showCamRunningInfo("Camera stopped", 1);

    return (true);
}

bool CamControl::StartCamDebug(void)
{
    COM_DEBUG(3, ("Func : %s\n", __FUNCTION__));
    if (CAM_STATUS_RUNNING == GetCamStatus()) {
        ReqCamStartDebug();
        return (true);
    }

    return (false);
}

void CamControl::run()
{
    HANDLE hSignImgQueueReady = 0;
    HANDLE hLaneImgQueueReady = 0;
    HANDLE hNotifyQueueReady = 0;
    HANDLE hRecoRetImgQueueReady = 0;

    hSignImgQueueReady = GetImageDisplayQueueDataReadyHandle(CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN);
    hLaneImgQueueReady = GetImageDisplayQueueDataReadyHandle(CAM_IMAGE_TYPE::IMAGE_TYPE_LANE);
    hRecoRetImgQueueReady = GetImageDisplayQueueDataReadyHandle(CAM_IMAGE_TYPE::IMAGE_TYPE_RECO_RET);
    hNotifyQueueReady = GetModuleControlNtyDataReadyHandle();

    if ((NULL == hSignImgQueueReady)    ||
        (NULL == hLaneImgQueueReady)    ||
        (NULL == hRecoRetImgQueueReady) ||
        (NULL == hNotifyQueueReady)) {
        COM_ERR("[ERR]ADC: CamControl::run, event error");
        return;
    }

    COM_DEBUG(5, ("ADC: receiving cam message thread start running"));

    HANDLE handles[MSG_HANDLE_MAX] = { 0 };
    handles[MSG_HANDLE_THREAD_QUIT] = m_hThreadQuitEvent;
    handles[MSG_HANDLE_NOTIFY_QUEUE_READY] = hNotifyQueueReady;
    handles[MSG_HANDLE_SIGN_IMG_QUEUE_READY] = hSignImgQueueReady;
    handles[MSG_HANDLE_LANE_IMG_QUEUE_READY] = hLaneImgQueueReady;
    handles[MSG_HANDLE_RECO_RET_IMG_QUEUE_READY] = hRecoRetImgQueueReady;

    DWORD dwRet = 0;
    int nCount = MSG_HANDLE_MAX;
    int nIndex = 0;
    while(m_bThreadRunFlag) {
        //COM_DEBUG(5, ("ADC: CamControl::run, 1 waiting multiple objects"));
        dwRet = WaitForMultipleObjects(nCount, &handles[0], FALSE, INFINITE);
        switch (dwRet) {
        case WAIT_TIMEOUT:
            COM_WARN("[WARN]ADC: CamControl::run, 1 WaitForMultipleObjects timeout");
            break;
        case WAIT_FAILED:
            COM_ERR("[ERR]ADC: CamControl::run, 1 WaitForMultipleObjects error");
            m_bThreadRunFlag = false;
            break;
        default:
            nIndex = dwRet - WAIT_OBJECT_0;
            processMsgHanlde(nIndex++);
            //同时检测其他的事件
            while(nIndex < nCount) { //nCount事件对象总数
                //COM_DEBUG(5, ("ADC: CamControl::run, 2 waiting multiple objects"));
                dwRet = WaitForMultipleObjects(nCount - nIndex, &handles[nIndex], false, 0);
                switch(dwRet) {
                case WAIT_TIMEOUT:
                    nIndex = nCount; //退出检测,因为没有被触发的对象了.
                    break;
                case WAIT_FAILED:
                    COM_ERR("[ERR]ADC: CamControl::run, 2 WaitForMultipleObjects error");
                    m_bThreadRunFlag = false;
                    break;
                default:
                    nIndex = nIndex + dwRet - WAIT_OBJECT_0;
                    processMsgHanlde(nIndex++);
                    break;
                }
            }
            break;
        }
    }

    COM_DEBUG(5, ("ADC: receiving cam message thread end running"));

    return;
}

void CamControl::processMsgHanlde(int index)
{
    //COM_DEBUG(5, ("CamControl::processMsgHanlde index[%d]", index));
    switch (index) {
    case (MSG_HANDLE_THREAD_QUIT):
        handleThreadQuitMsg();
        break;
    case (MSG_HANDLE_NOTIFY_QUEUE_READY):
        handleNotifyMsg();
        break;
    case (MSG_HANDLE_SIGN_IMG_QUEUE_READY):
        handleSignImgMsg();
        break;
    case (MSG_HANDLE_LANE_IMG_QUEUE_READY):
        handleLaneImgMsg();
        break;
    case (MSG_HANDLE_RECO_RET_IMG_QUEUE_READY):
        handleRecoRetImgMsg();
        break;
    default:
        break;
    }
}

void CamControl::handleThreadQuitMsg()
{
    COM_DEBUG(5, ("CamControl::handleThreadQuitMsg"));
    m_bThreadRunFlag = false;
}

void CamControl::handleNotifyMsg()
{
    char strBuff[512] = { 0 };

    IPC_MSG_TYPE* pMsg = NULL;
    do {
        //COM_DEBUG(5, ("CamControl::handleNotifyMsg, GetFrontModuleControlNtyFromQueueAndDonotWait"));
        pMsg = GetFrontModuleControlNtyFromQueueAndDonotWait();
        //COM_DEBUG(5, ("CamControl::handleNotifyMsg, pMsg %p", pMsg));
        if (NULL != pMsg) {
            switch (pMsg->msgId) {
            case (MSG_CAM_NTY_PROCESS_START):
                COM_DEBUG(3, ("CamCapture process started"));
                break;
            case (MSG_CAM_NTY_WORK_START):
                ChangeCamStatus(CAM_STATUS_RUNNING);
                COM_DEBUG(3, ("CamCapture start working"));
                emit showCamRunningInfo("Camera started", 2);
                StartCamDebug();
                break;
            case (MSG_CAM_NTY_IS_FINDING_DEVICE):
                COM_DEBUG(3, ("CamCapture is finding camera device"));
                break;
            case (MSG_CAM_NTY_FINDED_DEVICE_NUM):
                COM_DEBUG(3, ("CamCapture finded %d devices", pMsg->msg.cameraNum));
                break;
            case (MSG_CAM_NTY_LANE_RECO_RET):
                sprintf(strBuff, "Lane changed Event (%d)\nDistance to left line (%d)\nDistance to right line (%d)",
                        pMsg->msg.cam.lane.changeLaneEvent,
                        pMsg->msg.cam.lane.leftLineDistance,
                        pMsg->msg.cam.lane.rightLineDistance);
                emit showCamRunningInfo(strBuff, 2);
                break;
            default:
                break;
            }

            PopModuleControlNtyFromQueue(pMsg);
        }
    }while(NULL != pMsg);
}

void CamControl::handleSignImgMsg()
{
    CAM_IMAGE_TYPE* pImage = NULL;
    do {
        pImage = GetFrontImageFromDisplayQueueAndDonotWait(CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN);

        if (NULL != pImage) {
//            COM_DEBUG(5, ("ADC: image: imageType(%d) cols(%d) rows(%d)\n",
//                          pImage->image_type, pImage->cols, pImage->rows));
            emit camSignImgChanged(pImage->data, pImage->cols, pImage->rows);

            PopImageFromDisplayQueue(CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN, pImage);
        }
    }while(NULL != pImage);
}

void CamControl::handleLaneImgMsg()
{
    CAM_IMAGE_TYPE* pImage = NULL;
    do {
        pImage = GetFrontImageFromDisplayQueueAndDonotWait(CAM_IMAGE_TYPE::IMAGE_TYPE_LANE);
        if (NULL != pImage) {
//            COM_DEBUG(5, ("ADC: image: imageType(%d) cols(%d) rows(%d)\n",
//                          pImage->image_type, pImage->cols, pImage->rows));
            emit camLaneImgChanged(pImage->data, pImage->cols, pImage->rows);

            PopImageFromDisplayQueue(CAM_IMAGE_TYPE::IMAGE_TYPE_LANE, pImage);
        }
    }while(NULL != pImage);
}

void CamControl::handleRecoRetImgMsg()
{
    CAM_IMAGE_TYPE* pImage = NULL;
    do {
        pImage = GetFrontImageFromDisplayQueueAndDonotWait(CAM_IMAGE_TYPE::IMAGE_TYPE_RECO_RET);
        if (NULL != pImage) {
            COM_DEBUG(5, ("ADC: image: imageType(%d) cols(%d) rows(%d) size(%d) xDistance(%f) yDistance(%f) direction(%d)\n",
                          pImage->image_type, pImage->cols, pImage->rows, pImage->imgSize,
                          pImage->recognizeInfo.xDistance, pImage->recognizeInfo.yDistance, pImage->recognizeInfo.direction));
            emit camRecoRetImgChanged(pImage->data, pImage->cols, pImage->rows, pImage->recognizeInfo.direction, pImage->recognizeInfo.xDistance, pImage->recognizeInfo.yDistance);

            PopImageFromDisplayQueue(CAM_IMAGE_TYPE::IMAGE_TYPE_RECO_RET, pImage);
        }
    }while(NULL != pImage);
}

