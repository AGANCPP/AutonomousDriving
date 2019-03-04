#ifndef CAMCONTROL_H
#define CAMCONTROL_H

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QThread>
#include <QMutex>

#include "IpcAccess.h"

enum {
    CAM_STATUS_IDLE,
    CAM_STATUS_STARTING,
    CAM_STATUS_RUNNING,
    CAM_STATUS_ENDING
};

/******************************************************************************
* class define
******************************************************************************/
class CamControl : public QThread
{
    Q_OBJECT

public:
    CamControl();
    ~CamControl();

    int GetCamStatus(void);
    void ChangeCamStatus(int status_);

    bool StartCam(void);
    bool EndCam(void);
    bool StartCamDebug(void);

signals:
    void camSignImgChanged(uchar* PictureData, int width, int height);
    void camLaneImgChanged(uchar* PictureData, int width, int height);
    void camRecoRetImgChanged(uchar* PictureData, int width, int height, int direction, float xDistance, float yDistance);
    void showCamRunningInfo(const QString& info, int level);

protected:
    void run();

private:
    enum {
        MSG_HANDLE_THREAD_QUIT,
        MSG_HANDLE_NOTIFY_QUEUE_READY,
        MSG_HANDLE_SIGN_IMG_QUEUE_READY,
        MSG_HANDLE_LANE_IMG_QUEUE_READY,
        MSG_HANDLE_RECO_RET_IMG_QUEUE_READY,
        MSG_HANDLE_MAX
    };
    void processMsgHanlde(int index);
    void handleThreadQuitMsg();
    void handleNotifyMsg();
    void handleSignImgMsg();
    void handleLaneImgMsg();
    void handleRecoRetImgMsg();

private:
    int m_nCamStatus;
    TCHAR m_szCamCaptureProcess[512];
    STARTUPINFO m_siCamCaptureProcess;
    PROCESS_INFORMATION m_piCamCaptureProcess;

    volatile bool m_bThreadRunFlag;
    HANDLE m_hThreadQuitEvent;
};

#endif // CAMCONTROL_H
