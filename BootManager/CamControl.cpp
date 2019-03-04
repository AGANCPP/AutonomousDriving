// CamControl.cpp
#include <QtCore/QDebug>
#include "CamControl.h"
////#include "IpcAccess.h"

CamControl::CamControl()
{
    m_nCamStatus = CAM_STATUS_IDLE;
    lstrcpy(m_szCamCaptureProcess, TEXT("CamCapture/CamCapture.exe"));
    m_siCamCaptureProcess.cb = sizeof(m_siCamCaptureProcess);
    m_piCamCaptureProcess.hProcess = NULL;
}

int CamControl::GetCamStatus(void)
{
    return (m_nCamStatus);
}

void CamControl::ChangeCamStatus(int status_)
{
    qDebug("CamControl::ChangeCamStatus() is called. Change cam status from (%d) to (%d)", m_nCamStatus, status_);
    m_nCamStatus = status_;
}

bool CamControl::StartCam(void)
{
    qDebug("CamControl::StartCam() is called");
    BOOL bRetValue = FALSE;
    DWORD dwRetValue = 0;
    int camStatus = GetCamStatus();
    if (CAM_STATUS_IDLE != camStatus)
    {
        qWarning("CamControl::StartCam: Cam status error (%d)\n", camStatus);
        return (false);
    }
    ChangeCamStatus(CAM_STATUS_STARTING);
    // create CamCapture process
    if (NULL == m_piCamCaptureProcess.hProcess)
    {
        bRetValue = CreateProcess(NULL, m_szCamCaptureProcess, NULL, NULL, FALSE, 0, NULL, NULL, &m_siCamCaptureProcess, &m_piCamCaptureProcess);
        if (FALSE == bRetValue)
        {
            ChangeCamStatus(CAM_STATUS_IDLE);
            qCritical("CamControl::StartCam: Create process CamCapture error.");
            return (false);
        }
        qDebug("CamControl::StartCam: Create process CamCapture ok.");
    }
    else
    {
        qWarning("CamControl::StartCam: Process CamCapture is already opened.");
        return (false);
    }
    return (true);
}

bool CamControl::EndCam(void)
{
    qDebug("CamControl::EndCam is called");
    DWORD dwRetValue = 0;
    int camStatus = GetCamStatus();
    if (CAM_STATUS_RUNNING != camStatus)
    {
        qWarning("CamControl::EndCam: Cam status error (%d)\n", camStatus);
        return (false);
    }
    ChangeCamStatus(CAM_STATUS_ENDING);
    qDebug("Send camera end work message to CamCapture...\n");
    ////ReqCamEndWork();
    qDebug("Waiting CamCapture process exit...\n");
    dwRetValue = WaitForSingleObject(m_piCamCaptureProcess.hProcess, 20000);
    switch (dwRetValue)
    {
        case WAIT_OBJECT_0:
            qDebug("Waiting CamCapture process exit...ok\n");
            break;
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
            qCritical("Waiting CamCapture process exit...error\n");
            TerminateProcess(m_piCamCaptureProcess.hProcess, 0);
            break;
        default:
            break;
    }
    CloseHandle(m_piCamCaptureProcess.hProcess);
    m_piCamCaptureProcess.hProcess = NULL;
    ChangeCamStatus(CAM_STATUS_IDLE);
    return (true);
}

bool CamControl::StartCamDebug(void)
{
    qDebug("CamControl::StartCamDebug is called.");
    ChangeCamStatus(CAM_STATUS_RUNNING);
    ////ReqCamStartDebug();
    return (true);
}

bool CamControl::StartCamCapture(void)
{
    qDebug("CamControl::StartCamCapture is called.");
    ChangeCamStatus(CAM_STATUS_RUNNING);
    ////ReqCamStartCapture();
    return (true);
}
