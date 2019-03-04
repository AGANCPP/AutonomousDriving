
#include "controlthread.h"
#include "IpcMsgDef.h"
//#include "IpcAccess.h"
#include "CamControl.h"

extern CamControl g_camControl;
//VehControl g_vehControl;

ControlThreadSign::ControlThreadSign()
{
    //    moveToThread(this);
}


ControlThreadSign::~ControlThreadSign()
{
    qDebug() << "test";
}

void ControlThreadSign::run()
{
    static int i;
    CAM_IMAGE_TYPE* pimage = NULL;
    QTime m_time;
    int iPreTime = 0, iCurrTime = 0;
    while (true)
    {
        //qDebug() << "count" << i++;
        ////pimage = GetFrontImageFromDisplayQueue(CAM_IMAGE_TYPE::IMAGE_TYPE_SIGN);
        if (NULL != pimage)
        {
            qDebug("Receive image type(%d) cols(%d) rows(%d) step(%d) data_size(%d)",
                   pimage->image_type, pimage->cols, pimage->rows, pimage->step, pimage->imgSize);
            //m_imageQueue.PopFront();
            iCurrTime = m_time.elapsed();
            qDebug("m_time.elapsed sign:%d", iCurrTime - iPreTime);
            iPreTime = iCurrTime;
            emit notifyShowLaveOneScreen(pimage->data, pimage->cols, pimage->rows);
            ////PopImageFromDisplayQueue(pimage->image_type, pimage);
        }
        else
            qCritical("Receive image err!");
    }
}

void ControlThreadSign::getData()
{
}

ControlThreadLane::ControlThreadLane()
{
    //    moveToThread(this);
}

ControlThreadLane::~ControlThreadLane()
{
}

void ControlThreadLane::run()
{
    static int i;
    CAM_IMAGE_TYPE* pimage = NULL;
    QTime m_time;
    int iPreTime = 0, iCurrTime = 0;
    while (true)
    {
        //qDebug() << "count" << i++;
        ////pimage = GetFrontImageFromDisplayQueue(CAM_IMAGE_TYPE::IMAGE_TYPE_LANE);
        if (NULL != pimage)
        {
            qDebug("Receive image type(%d) cols(%d) rows(%d) step(%d) data_size(%d)",
                   pimage->image_type, pimage->cols, pimage->rows, pimage->step, pimage->imgSize);
            iCurrTime = m_time.elapsed();
            qDebug("m_time.elapsed lane:%d", iCurrTime - iPreTime);
            iPreTime = iCurrTime;
            emit notifyShowLaveTwoScreen(pimage->data,  pimage->cols, pimage->rows);
            ////PopImageFromDisplayQueue(pimage->image_type, pimage);
        }
        else
            qCritical("Receive image err!");
    }
}

void ControlThreadLane::getData()
{
}


ControlThreadRecvCamNotify::ControlThreadRecvCamNotify()
{
    //    moveToThread(this);
}

ControlThreadRecvCamNotify::~ControlThreadRecvCamNotify()
{
    qDebug() << "test";
}


void ControlThreadRecvCamNotify::run()
{
    while (true)
    {
        IPC_MSG_TYPE* pMsg = NULL;////GetFrontModuleControlNtyFromQueue();
        if (NULL != pMsg)
        {
            switch (pMsg->msgId)
            {
                case (MSG_CAM_NTY_PROCESS_START):
                    qDebug("RecvCamNotify::HandleMessage:CamCapture process started\n");
                    g_camControl.ChangeCamStatus(CAM_STATUS_STARTING);
                    emit sigProcessStartSuccess();
                    break;
                case (MSG_CAM_NTY_WORK_START):
                    g_camControl.ChangeCamStatus(CAM_STATUS_RUNNING);
                    qDebug("RecvCamNotify::HandleMessage:CamCapture start working\n");
                    break;
                case (MSG_CAM_NTY_IS_FINDING_DEVICE):
                    qDebug("RecvCamNotify::HandleMessage:CamCapture is finding camera device\n");
                    break;
                case (MSG_CAM_NTY_FINDED_DEVICE_NUM):
                {
                    qDebug("RecvCamNotify::CamCapture finded %d devices\n", pMsg->msg.cameraNum);
                    QString strCamNumber = QString::number(pMsg->msg.cameraNum);
                    emit sigDisplayDeviceNumber((const QString)strCamNumber);
                }
                break;
                case MSG_CAM_NTY_RECOG_TYPE:
                {
                    qDebug("RecvCamNotify::CamCapture recognize type:%d\n", pMsg->msg.cam.recogType);
                    QString strRecogType = QString::number(pMsg->msg.cam.recogType);
                    emit sigDisplayRecogType((const QString)strRecogType);
                }
                break;
                case MSG_CAM_NTY_GRAY_VALUE:
                    qDebug("RecvCamNotify::CamCapture gray value:%d\n", pMsg->msg.cam.grayValue);
                    emit sigDisplayGrayValue(pMsg->msg.cam.grayValue);
                    break;
                case MSG_CAM_NTY_EXPOSURE_TIME:
                {
                    qDebug("RecvCamNotify::CamCapture exposure time:%d\n", pMsg->msg.cam.exposureTime);
                    //QString strExposureTime = QString::number(pMsg->msg.cam.exposureTime);
                    //emit sigDisplayExposureTime((const QString)strExposureTime);
                    emit sigDisplayExposureTime(pMsg->msg.cam.exposureTime);
                }
                break;
                case (MSG_VEH_NTY_WORK_START):
                    qDebug("Vehicle control start working\n");
                    //g_vehControl.ChangeStatus(VEH_STATUS_RUNNING);
                    break;
                case (MSG_CAM_NTY_VERSION):
                    // Version msg
                    qDebug("RecvCamNotify::CamCapture Version:%s\n", pMsg->msg.version);
                    emit sigDisplayCamVersion(pMsg->msg.version);
                    break;
                default:
                    //                qDebug("RecvCamNotify::CamCapture, default msg: %d\n", pMsg->msgId);
                    break;
            }
            ////PopModuleControlNtyFromQueue(pMsg);
        }
    }
}

void ControlThreadRecvCamNotify::getData()
{
}

ControlThreadCamera::ControlThreadCamera()
{
    //    moveToThread(this);
    m_hEventStart = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventStart)
        qCritical("Creat event failed in ControlThreadCamera");
    m_hEventEnd = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventEnd)
        qCritical("Creat event failed in ControlThreadCamera");
}

ControlThreadCamera::~ControlThreadCamera()
{
    qDebug() << "ControlThreadCamera::~ControlThreadCamera()";
    if (NULL != m_hEventStart)
        CloseHandle(m_hEventStart);
    if (NULL != m_hEventEnd)
        CloseHandle(m_hEventEnd);
}

void ControlThreadCamera::run()
{
    DWORD dwRet = 0;
    int nIndex = 0;
    HANDLE hCameraControl[2] = {m_hEventStart, m_hEventEnd};
    while (true)
    {
        dwRet = ::WaitForMultipleObjects(2, hCameraControl, FALSE, INFINITE);
        switch (dwRet)
        {
            case WAIT_TIMEOUT:
                //nIndex = nCount; //退出检测,因为没有被触发的对象了.
                qCritical() << "WAIT_TIMEOUT dwRet:" << dwRet;
                continue;
            case WAIT_FAILED:
                qCritical() << "WAIT_FAILED dwRet:" << dwRet;
                return;
            default:
            {
                nIndex = dwRet - WAIT_OBJECT_0;
                if (0 == nIndex)
                {
                    if (!g_camControl.StartCam())
                    {
                        //addPlainText("onLaveOpenBtnTouched: CamCapture process open failed, check if file exists or already opened\n");
                        qDebug("CamCapture process open failed, check if file exists or already opened");
                        ::ResetEvent(m_hEventStart);
                        emit sigProcessStartFailure();
                        continue;
                    }
                    else
                    {
                        qDebug("CamCapture process open succeeded");
                        ::ResetEvent(m_hEventStart);
                    }
                }
                else if (1 == nIndex)
                {
                    if (!g_camControl.EndCam())
                    {
                        qDebug("CamCapture process end failed.");
                        ::ResetEvent(m_hEventEnd);
                        emit sigProcessEndFailure();
                        continue;
                    }
                    else
                    {
                        qDebug("CamCapture process end succeeded.");
                        ::ResetEvent(m_hEventEnd);
                        emit sigProcessEndSuccess();
                    }
                }
            }
            break;
        }
    }
}

void ControlThreadCamera::getData()
{
}

void ControlThreadCamera::processStartCam()
{
    qDebug() << "ControlThreadCamera::processStartCam()";
    ::SetEvent(m_hEventStart);
}

void ControlThreadCamera::processEndCam()
{
    qDebug() << "ControlThreadCamera::processEndCam()";
    ::SetEvent(m_hEventEnd);
}
