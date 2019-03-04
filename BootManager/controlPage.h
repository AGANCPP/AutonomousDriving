#ifndef CONTROLPAGE_H
#define CONTROLPAGE_H

#include <QtCore/QtCore>
#include "controlthread.h"
#include "showscreen.h"
#include "roadscreen.h"


#define COLOR_BUTTON QColor(21, 49, 82, 255)
#define COLOR_GROUPBOX QColor(21, 49, 82, 255)
#define COLOR_GROUPBOX_TITLE QColor(255, 255, 255, 255)

namespace Ui
{
    class ControlPage;
}

/*******20150121(begin)*********/
typedef enum _tagCamerLog
{
    CAMERA_LOG_INFO = 0,
    CAMERA_TEXTLOG_ON,
    CAMERA_TEXTLOG_OFF,
    CAMERA_IMAGELOG_ON,
    CAMERA_IMAGELOG_OFF,
    CAMERAINFO_LOG_INVALID
} CameraLogStatus;
/******20150121(end)**********/

/******20150211(begin)******/
typedef enum _tagNaviStatus
{
    NAVI_INIT = 0,
    NAVI_OPENED,
    NAVI_CLOSED,
    NAVI_INVALID
} NaviStatus;
/******20150211(end)********/


class ControlPage : public QWidget
{
    Q_OBJECT
public:
    ControlPage(QWidget* parent = 0);
    ~ControlPage();
    void addPlainText(const QString& text);

    void init();
    void iconInit();
    void getInfo();
    void disableButtonInGroup(QPushButton* buttonDisabled1, QPushButton* buttonDisabled2);
    /****add by runz(begin) 2015.01.12*********/
    void enableButton(QPushButton* buttonEnable);
    void disableButton(QPushButton* buttonDisable);
    /****add by runz(end) 2015.01.12*********/
    void switchButtonInGroup(QPushButton* buttonEnabled, QPushButton* buttonDisabled);
    void switchSliderAndSpinBoxInGroup(QSlider* valueSlider, QSpinBox* valueBox, BOOL bEnabled);

signals:
    void sigSendStartCC();
    void sigSendEndCC();
    void addExpValue(int value);
    void plusExpValue(int value);

public slots:
    void onNaviOpenBtnTouched(void);
    void onNaviCloseBtnTouched(void);
    void onLaveOpenBtnTouched(void);
    void onLaveOpenBtnSucceeded(void);
    void onLaveOpenBtnFailed(void);
    void onLaveCloseBtnTouched(void);
    void onLaveCloseBtnSucceeded(void);
    void onLaveCloseBtnFailed(void);
    void onLogOpenBtnTouched(void);
    void onLogCloseBtnTouched(void);


    /****20150227(begin)*********/
    void handleCamImage(int check);
    /****20150227(end)***********/

    void onReqCamEnableAutoExposure();
    void onReqCamDisableAutoExposure();

    void setGrayValueAccept();
    void setGrayValueIgnore();
    void setGrayValueBox(int iValue);
    //Start Modify 2014-12-22
    void setGrayValue();

    /***20150122*****/
    void setExpValue();                //send exposure value to camer through slider
    void setExpValueBox(int iValue);  //receive exposure value from camera
    void onReqAddExpValue();  //add exposuer value through keybord
    void onReqPlusExpValue(); //plus exposuer value through keybord
    void setExpValueAccept();
    void setExpValueIgnore();
    /***20150122*****/

    // Add 2015-01-05
    void setTrafficSignIcon(int iType);
    void setLaneEventIcon(int iLane);

    //    void displayDeviceNumber();
    //    void displayGrayValue();
    //    void displayExposureTime();
    //End modify 2014-12-22

    /*******20150113(begin)******/
    void handleEnterPark();
    void handleLeavePark();
    /*******20150113(end)******/
    void showTime();


private:
    Ui::ControlPage* ui;

    ControlThreadSign* controlThreadSign;
    ControlThreadLane* controlThreadLane;
    ControlThreadRecvCamNotify* controlThreadRecvCamNotify;
    ControlThreadCamera* controlThreadCamera;

    //ShowScreen* showNaviScreen;
    ShowScreen* showLaveOneScreen;
    ShowScreen* showLaveTwoScreen;
    RoadScreen* showRoadScreen;

    QProcess* m_pProcessCameraCan;
    QProcess* m_pProcessNavi;

    int m_nNaviWidth;
    int m_nNaviHeight;
    int m_nLaneWidth;
    int m_nLaneHeight;
    int m_nSignWidth;
    int m_nSignHeight;
    int m_nRoadWidth;
    int m_nRoadHeight;

    int m_nScreenWidth;
    int m_nScreenHeight;

    int g_nActScreenW;
    int g_nActScreenH;

    float m_nScaleW;
    float m_nScaleH;

    bool b_isCamCapture;

    //void sendNaviLogStartCmd();
    void sendNaviLogStartCmd();
    void sendNaviLogStopCmd();
    /****add by runz(begin) 2015.01.06*********/
    void sendCamLogStartCmd();
    void sendCamLogStopCmd();
    /****add by runz(end) 2015.01.06*********/
    /******20150121(begin)**********/
    void sendCamImageStartCmd();
    void sendCamImageStopCmd();
    /******20150121(end)**********/

    uchar m_ucAcceptGrayValue;
    uchar m_uAcceptExpValue;

    /*****20150206(begin)*****/
    QImage* m_icon_left;
    QImage* m_icon_right;

    QImage* m_icon_blank;

    QImage* m_icon_guide;
    QImage* m_icon_traffic;
    QImage* m_icon_call;
    /*****20150206(end)*******/

};

#endif // PAGES_H
