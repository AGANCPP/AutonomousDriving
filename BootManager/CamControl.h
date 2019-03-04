// CamControl.h
#ifndef CAM_CONTROL_H
#define CAM_CONTROL_H
#include <windows.h>

enum
{
    CAM_STATUS_IDLE,
    CAM_STATUS_STARTING,
    CAM_STATUS_RUNNING,
    CAM_STATUS_ENDING,
};

class CamControl
{
public:
    CamControl();
    int GetCamStatus(void);
    void ChangeCamStatus(int status_);

    bool StartCam(void);
    bool EndCam(void);
    bool StartCamDebug(void);
    bool StartCamCapture(void);

private:
    int m_nCamStatus;
    TCHAR m_szCamCaptureProcess[128];
    STARTUPINFO m_siCamCaptureProcess;
    PROCESS_INFORMATION m_piCamCaptureProcess;
};

#endif // CAM_CONTROL_H

