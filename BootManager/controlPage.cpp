
#include "controlPage.h"
#include "rootviewer.h"
#include "BaseDefine.h"
#include "systemlog.h"
#include "safeDelete.h"
#include "CamControl.h"
//#include "main.h"
//#include "IpcAccess.h"
#include "ui_controlpage.h"

QString g_logPath;
CamControl g_camControl;
ControlThreadRecvCamNotify* g_controlThreadRecvCamNotify = NULL;

/******20150211(begin)**********/
int navi_status = 0;
/******20150211(end)************/

ControlPage::ControlPage(QWidget* parent)
    : QWidget(parent),
      controlThreadSign(NULL),
      controlThreadLane(NULL),
      controlThreadRecvCamNotify(NULL),
      controlThreadCamera(NULL),
      showLaveOneScreen(NULL),
      showLaveTwoScreen(NULL),
      showRoadScreen(NULL),
      /*****20150206(begin)******/
      m_icon_left(NULL),
      m_icon_right(NULL),

      m_icon_blank(NULL),

      m_icon_guide(NULL),
      m_icon_traffic(NULL),
      m_icon_call(NULL),
      /*****20150206(end)******/

      m_nNaviWidth(CONFIG_DEF_NAVI_WIDTH),
      m_nNaviHeight(CONFIG_DEF_NAVI_HEIGHT),
      m_nLaneWidth(CONFIG_DEF_LANE_WIDTH),
      m_nLaneHeight(CONFIG_DEF_LANE_HEIGHT),
      m_nSignWidth(CONFIG_DEF_SIGN_WIDTH),
      m_nSignHeight(CONFIG_DEF_SIGN_HEIGHT),
      m_nRoadWidth(CONFIG_DEF_ROAD_WIDTH),
      m_nRoadHeight(CONFIG_DEF_ROAD_HEIGHT),
      m_nScreenWidth(CONFIG_DEF_SCR_WIDTH),
      m_nScreenHeight(CONFIG_DEF_SCR_HEIGHT),
      g_nActScreenW(0),
      g_nActScreenH(0),
      b_isCamCapture(false),
      ui(new Ui::ControlPage)
{
    ui->setupUi(this);
    /*****20150206(begin)********/
    iconInit();
    /*****20150206(end)********/
    init();
}

ControlPage::~ControlPage()
{
    qDebug("~ControlPage");
    SAFE_DELETE(controlThreadSign);
    SAFE_DELETE(controlThreadLane);
    SAFE_DELETE(controlThreadRecvCamNotify);
    SAFE_DELETE(controlThreadCamera);
    SAFE_DELETE(showLaveOneScreen);
    SAFE_DELETE(showLaveTwoScreen);
    SAFE_DELETE(showRoadScreen);
    if (NULL != m_pProcessCameraCan)
    {
        m_pProcessCameraCan->close();
        SAFE_DELETE(m_pProcessCameraCan);
    }
    if (NULL != m_pProcessNavi)
    {
        m_pProcessNavi->close();
        SAFE_DELETE(m_pProcessNavi);
    }
    /*****20150206(begin)******/
    SAFE_DELETE(m_icon_left);
    SAFE_DELETE(m_icon_right);
    SAFE_DELETE(m_icon_blank);
    SAFE_DELETE(m_icon_guide);
    SAFE_DELETE(m_icon_traffic);
    SAFE_DELETE(m_icon_call);
    /*****20150206(end)********/
    delete ui;
}

/*****20150206(begin)********/
void ControlPage::iconInit()
{
    m_icon_left = new QImage(tr("./image/icon_left.png"));
    *m_icon_left = m_icon_left->scaled(50, 50, Qt::KeepAspectRatio);
    m_icon_right = new QImage(tr("./image/icon_right.png"));
    *m_icon_right = m_icon_right->scaled(50, 50, Qt::KeepAspectRatio);
    m_icon_blank = new QImage(tr("./image/icon_blank.png"));
    *m_icon_blank = m_icon_blank->scaled(50, 50, Qt::KeepAspectRatio);
    m_icon_guide = new QImage(tr("./image/icon_guide.png"));
    *m_icon_guide = m_icon_guide->scaled(50, 50, Qt::KeepAspectRatio);
    m_icon_traffic = new QImage(tr("./image/icon_trffic.png"));
    *m_icon_traffic = m_icon_traffic->scaled(50, 50, Qt::KeepAspectRatio);
    m_icon_call = new QImage(tr("./image/icon_call.png"));
    *m_icon_call = m_icon_call->scaled(50, 50, Qt::KeepAspectRatio);
}
/*****20150206(end)*********/


void ControlPage::init()
{
    m_ucAcceptGrayValue = 1;
    m_uAcceptExpValue = 1;
    m_pProcessCameraCan = NULL;
    m_pProcessNavi = NULL;
    QShortcut*  m_Ctl_SetOnAutoExposure_Accel = new QShortcut(QKeySequence(tr("Ctrl+E")), this);
    connect(m_Ctl_SetOnAutoExposure_Accel, SIGNAL(activated()), this, SLOT(onReqCamEnableAutoExposure()));
    QShortcut*  m_Ctl_SetOffAutoExposure_Accel = new QShortcut(QKeySequence(tr("Ctrl+D")), this);
    connect(m_Ctl_SetOffAutoExposure_Accel, SIGNAL(activated()), this, SLOT(onReqCamDisableAutoExposure()));
    /*****20150122********/
    QShortcut*  m_Ctl_addExpValue = new QShortcut(QKeySequence(tr("Ctrl+A")), this);
    connect(m_Ctl_addExpValue, SIGNAL(activated()), this, SLOT(onReqAddExpValue()));
    QShortcut*  m_Ctl_plusExpValue = new QShortcut(QKeySequence(tr("Ctrl+P")), this);
    connect(m_Ctl_plusExpValue, SIGNAL(activated()), this, SLOT(onReqPlusExpValue()));
    /*****20150122********/
    switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, FALSE);
    //show min,s,ms in controlPage
    QTimer* timer;
    //ui->lcdNumber->setNumDigits(12);
    ui->lcdNumber->display(QTime::currentTime().toString("hh:mm:ss:zzz"));
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    //timer->start(1000);
    timer->start(1);
    // Create thread
    controlThreadSign = new ControlThreadSign();
    controlThreadSign->start();
    controlThreadLane = new ControlThreadLane();
    controlThreadLane->start();
    controlThreadRecvCamNotify = new ControlThreadRecvCamNotify();
    controlThreadRecvCamNotify->start();
    controlThreadCamera = new ControlThreadCamera();
    controlThreadCamera->start();
    // About page use.
    g_controlThreadRecvCamNotify = controlThreadRecvCamNotify;
    // Get screen info(width/height)
    getInfo();
    // Connect signal and slot
    QWidget::connect(ui->NaviOpenBtn,  SIGNAL(clicked()), this, SLOT(onNaviOpenBtnTouched()));
    QWidget::connect(ui->NaviCloseBtn, SIGNAL(clicked()), this, SLOT(onNaviCloseBtnTouched()));
    QWidget::connect(ui->LaveOpenBtn,  SIGNAL(clicked()), this, SLOT(onLaveOpenBtnTouched()));
    QWidget::connect(this,  SIGNAL(sigSendStartCC()), controlThreadCamera, SLOT(processStartCam()));
    QWidget::connect(ui->LaveCloseBtn, SIGNAL(clicked()), this, SLOT(onLaveCloseBtnTouched()));
    QWidget::connect(this,  SIGNAL(sigSendEndCC()), controlThreadCamera, SLOT(processEndCam()));
    //navi¨®?camera¦Ì?log?a??¨®?1?¡À?
    QWidget::connect(ui->logOpenBtn,   SIGNAL(clicked()), this, SLOT(onLogOpenBtnTouched()));
    QWidget::connect(ui->logCloseBtn,  SIGNAL(clicked()), this, SLOT(onLogCloseBtnTouched()));
    //image¦Ì?¡À¡ê¡ä?
    QWidget::connect(ui->imageCheckBox, SIGNAL(stateChanged(int)), this, SLOT(handleCamImage(int)));
    /******20150121(begin)**********/
    //QWidget::connect(ui->logBtnCam, SIGNAL(clicked()), this, SLOT(handleCameraLog()));
    //QWidget::connect(ui->imageBtnCam, SIGNAL(clicked()), this, SLOT(handleCameraImage()));
    /******20150121(end)**********/
    connect(controlThreadRecvCamNotify, SIGNAL(sigDisplayDeviceNumber(const QString&)), ui->deviceNumberLabel, SLOT(setText(const QString&)));
    connect(controlThreadRecvCamNotify, SIGNAL(sigProcessStartSuccess()), this, SLOT(onLaveOpenBtnSucceeded()));
    connect(controlThreadCamera, SIGNAL(sigProcessStartFailure()), this, SLOT(onLaveOpenBtnFailed()));
    connect(controlThreadCamera, SIGNAL(sigProcessEndSuccess()), this, SLOT(onLaveCloseBtnSucceeded()));
    connect(controlThreadCamera, SIGNAL(sigProcessEndFailure()), this, SLOT(onLaveCloseBtnFailed()));
    //Start Modify 2014-12-22
    connect(ui->grayValueSlider, SIGNAL(sliderReleased()), this, SLOT(setGrayValue()));
    connect(ui->grayValueSlider, SIGNAL(sliderReleased()), this, SLOT(setGrayValueAccept()));
    connect(ui->grayValueSlider, SIGNAL(sliderPressed()), this, SLOT(setGrayValueIgnore()));
    /***********20150122(begin)*************/
    switchSliderAndSpinBoxInGroup(ui->expValueSlider, ui->expValueBox, FALSE);
    connect(ui->expValueSlider, SIGNAL(sliderReleased()), this, SLOT(setExpValue()));
    connect(ui->expValueSlider, SIGNAL(sliderReleased()), this, SLOT(setExpValueAccept()));
    connect(ui->expValueSlider, SIGNAL(sliderPressed()), this, SLOT(setExpValueIgnore()));
    connect(ui->expValueSlider, SIGNAL(valueChanged(int)), ui->expValueBox, SLOT(setValue(int)));
    connect(ui->expValueBox, SIGNAL(valueChanged(int)), ui->expValueSlider, SLOT(setValue(int)));
    /***********20150122(end)*************/
    //Start Modify 2014-12-22
    //Start Modify 2014-12-23
    connect(ui->grayValueSlider, SIGNAL(valueChanged(int)), ui->grayValueBox, SLOT(setValue(int)));
    connect(ui->grayValueBox, SIGNAL(valueChanged(int)), ui->grayValueSlider, SLOT(setValue(int)));
    //End Modify 2014-12-23
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_LANEDISTANCE_LD_RD_X(const QString&)), ui->leftDistanceEditer, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_LANEDISTANCE_LD_RD_Y(const QString&)), ui->rightDistanceEditer, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_LANECHANGE_LEFT_RIGHT(int)), this, SLOT(setLaneEventIcon(int)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_TRAFFICSIGNAL_XYZ_L(const QString&)), ui->distance_XEditer, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_TRAFFICSIGNAL_XYZ_R(const QString&)), ui->distance_YEditer, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_TRAFFICSIGNAL_TYPE(int)), this, SLOT(setTrafficSignIcon(int)));
    /*******20150113(begin) show HighWay: in controlPage******/
    QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
    int opgDebug = iniSettings.value("About/OpgVer").toInt();
    if (1 == opgDebug) //opg version
    {
        ui->trafficRecgTypeLabel_5->setText("HighWay : NG");
        ////connect(g_strMyApplication, SIGNAL(MsgMobileNavigator_EnterPark()), this, SLOT(handleEnterPark()));
        ////connect(g_strMyApplication, SIGNAL(MsgMobileNavigator_LeavePark()), this, SLOT(handleLeavePark()));
    }
    else  //other version
    {
        //do not show enter/highway info in BootManager
    }
    /*******20150113(end)******/
}

void ControlPage::setTrafficSignIcon(int iType)
{
    switch (iType)
    {
        case TS_TRAFFIC_GUIDEBOARD:
            ui->trafficSignLabel->setPixmap(QPixmap::fromImage(*m_icon_guide));
            break;
        case TS_REGU_RATE_LIMIT:
            ui->trafficSignLabel->setPixmap(QPixmap::fromImage(*m_icon_traffic));
            break;
        case TS_UTILITY_EMERGENCY_CALL:
            ui->trafficSignLabel->setPixmap(QPixmap::fromImage(*m_icon_call));
            break;
        default:
            ui->trafficSignLabel->setPixmap(QPixmap::fromImage(*m_icon_blank));
            break;
    }
    qDebug() << "ControlPage::setTrafficSignIcon() :" << iType;
}

void ControlPage::setLaneEventIcon(int iLane)
{
    switch (iLane)
    {
        case 1:
            ui->laneEventLable->setPixmap(QPixmap::fromImage(*m_icon_left));
            break;
        case 2:
            ui->laneEventLable->setPixmap(QPixmap::fromImage(*m_icon_right));
            break;
        default:
            ui->laneEventLable->setPixmap(QPixmap::fromImage(*m_icon_blank));
            break;
    }
    qDebug() << "ControlPage::setLaneEventIcon() :" << iLane;
}

void ControlPage::setGrayValueAccept()
{
    m_ucAcceptGrayValue = 1;
    qDebug() << "ControlPage::setGrayValueAccept()" << m_ucAcceptGrayValue;
}

void ControlPage::setGrayValueIgnore()
{
    m_ucAcceptGrayValue = 0;
    qDebug() << "ControlPage::setGrayValueIgnore()" << m_ucAcceptGrayValue;
}


void ControlPage::sendNaviLogStartCmd()
{
    //switchButtonInGroup(ui->logCloseBtn, ui->logOpenBtn);
    //QString strPath1 = QCoreApplication::applicationFilePath();
    //qDebug()<<"Application Path:"<<strPath1;
    //QString runDirPath = QDir::currentPath();
    //qDebug()<<"Current Dir Path:"<<runDirPath;
    //QString logPath = "\\log\\";
    //g_strMobileNavigatorLogPath = runDirPath + logPath;
    WCHAR wcLogPath[512] = {0};
    //g_strMobileNavigatorLogPath.toWCharArray(wcLogPath);
    g_logPath.toWCharArray(wcLogPath);
    //qDebug()<<"Current Dir Path:"<<g_strMobileNavigatorLogPath;
    //qDebug()<<"str_len:"<<g_strMobileNavigatorLogPath.length();
    qDebug() << "sendNaviLogStartCmd : Current Dir Path:" << g_logPath;
    qDebug() << "sendNaviLogStartCmd : str_len:" << g_logPath.length();
    HWND hwndNavigator = NULL;
    hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
    if (hwndNavigator != NULL)
    {
        COPYDATASTRUCT struCopyData;
        struCopyData.dwData = WM_LOG_PATH;
        struCopyData.lpData = (void*)wcLogPath;
        //struCopyData.cbData = g_strMobileNavigatorLogPath.length()*2;//wcslen(g_strMobileNavigatorLogPath)*sizeof(kn_char);
        struCopyData.cbData = g_logPath.length() * 2; //wcslen(g_strMobileNavigatorLogPath)*sizeof(kn_char);
        qDebug() << "LogPath>struCopyData.dwData:" << struCopyData.dwData
                 << "; struCopyData.lpData:" << struCopyData.lpData
                 << "; struCopyData.cbData:" << struCopyData.cbData;
        ::SendMessage(hwndNavigator, WM_COPYDATA, (WPARAM)hwndNavigator, (LPARAM)&struCopyData);
        struCopyData.dwData = WM_START_LOG;
        ::SendMessage(hwndNavigator, WM_COPYDATA, (WPARAM)hwndNavigator, (LPARAM)&struCopyData);
        addPlainText("onLogOpenBtnTouched\n");
    }
    else
    {
        qDebug() << "Can't find Mobile Navigator.Please open it.";
        addPlainText("Can't find Mobile Navigator.\n");
    }
}

void ControlPage::sendNaviLogStopCmd()
{
    HWND hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
    if (hwndNavigator != NULL)
    {
        COPYDATASTRUCT struCopyData;
        struCopyData.dwData = WM_STOP_LOG;
        ::SendMessage(hwndNavigator, WM_COPYDATA, (WPARAM)hwndNavigator, (LPARAM)&struCopyData);
    }
    else
        addPlainText("Mobile Navigator has closed.\n");
    addPlainText("onLogCloseBtnTouched\n");
}



/****add by runz(begin) 2015.01.06*********/

void ControlPage::sendCamLogStartCmd()
{
    //QString strPath1 = QCoreApplication::applicationFilePath();
    //qDebug()<<"Application Path:"<<strPath1;
    //QString runDirPath = QDir::currentPath();
    //qDebug()<<"Current Dir Path:"<<runDirPath;
    //QString logPath = "\\log";
    //g_strCamCaptureLogPath = runDirPath + logPath;
    //change QString to char*
    char* path = NULL;
    //QByteArray ba = g_strCamCaptureLogPath.toLatin1();
    QByteArray ba = g_logPath.toLatin1();
    path = ba.data();
    //path = strdup(ba.data());
    qDebug() << "sendCamLogStartCmd() : Current Dir Path:" << g_logPath;
    qDebug() << "sendCamLogStartCmd() : str_len:" << g_logPath.length();
    HWND hwndCam = NULL;
    hwndCam = ::FindWindow(NULL, TEXT("Lane"));
    if (hwndCam != NULL)
    {
        ////ReqCamSetLogPath(path);
        ////ReqCamStartLog();
        addPlainText("start to save camera log\n");
    }
    else
    {
        qDebug() << "Can't find Camera.Please open it.";
        addPlainText("Can't find Camera.\n");
    }
}


void ControlPage::sendCamLogStopCmd()
{
    HWND hwndCam = ::FindWindow(NULL, TEXT("Lane"));
    if (hwndCam != NULL)
    {
        ////ReqCamEndLog();
        addPlainText("stop to save camera log\n");
    }
    else
        addPlainText("Camera has closed.\n");
}
/****add by runz(begin) 2015.01.06*********/


void ControlPage::onNaviOpenBtnTouched(void)
{
    HWND hwndCamCan = ::FindWindow(NULL, TEXT("CameraCAN"));
    if (NULL == m_pProcessCameraCan || NULL == hwndCamCan)
    {
        QString process_path = "CameraCAN/release/CameraCAN.exe";
        QFileInfo qFileInfo_Navi;
        qFileInfo_Navi.setFile(process_path);
        if (qFileInfo_Navi.exists())
        {
            m_pProcessCameraCan = new QProcess(this);
            m_pProcessCameraCan->start(process_path);
            addPlainText("onNaviOpenBtnTouched: open CameraCan process\n");
        }
        else
        {
            qCritical("onNaviOpenBtnTouched: CameraCan process file does not exist. Please copy one.");
            addPlainText("onNaviOpenBtnTouched: CameraCan process file does not exist. Please copy one.\n");
            return;
        }
    }
    else
        addPlainText("onNaviOpenBtnTouched: CameraCan process is already opened\n");
    HWND hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
    if (NULL == m_pProcessNavi || NULL == hwndNavigator)
    {
        QString process_path = "Navigator/releasevc/Navigator.exe";
        QFileInfo qFileInfo_Navi;
        qFileInfo_Navi.setFile(process_path);
        if (qFileInfo_Navi.exists())
        {
            m_pProcessNavi = new QProcess(this);
            m_pProcessNavi->start(process_path);
            switchButtonInGroup(ui->NaviCloseBtn, ui->NaviOpenBtn);
            /*****20150227(begin)**********/
            enableButton(ui->logOpenBtn);
            /*****20150227(end)**********/
            addPlainText("onNaviOpenBtnTouched: open Navi process\n");
        }
        else
        {
            qCritical("onNaviOpenBtnTouched: Navi process file does not exist. Please copy one.");
            addPlainText("onNaviOpenBtnTouched: Navi process file does not exist. Please copy one.\n");
            //switchButtonInGroup(ui->NaviOpenBtn, ui->NaviCloseBtn);
            return;
        }
    }
    else
        addPlainText("onNaviOpenBtnTouched: Navi process is already opened\n");
    //connect(getRootViewer(), SIGNAL(notifyCloseAll()), this, SLOT(onNaviCloseBtnTouched()));
    connect(getRootViewer(), SIGNAL(notifyCloseNavi()), this, SLOT(onNaviCloseBtnTouched()));
    Sleep(100);
    hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
    int i = 0;
    while (NULL == hwndNavigator && i < 50)
    {
        Sleep(100);
        hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
        i++;
    }
    qDebug() << "hwndNavigator:" << hwndNavigator ;
    if (hwndNavigator)
    {
        //::MoveWindow(hwndNavigator,0,0,800,480,TRUE);
        ::MoveWindow(hwndNavigator, 0, 0, 1280, 1024, TRUE);
        qDebug() << "MoveWindow: " << i ;
    }
    else
        qDebug() << "Navi is not started:" << i ;
    hwndCamCan = ::FindWindow(NULL, TEXT("CameraCAN"));
    ::ShowWindow(hwndCamCan, SW_MINIMIZE);
    /****20150211(begin)***********/
    navi_status = NAVI_OPENED;
    /****20150211(end)***********/
}

void ControlPage::onNaviCloseBtnTouched(void)
{
    //if(NULL != m_pProcessNavi)
    //{
    //    m_pProcessNavi->close();
    //    m_pProcessNavi = NULL;
    //    addPlainText("onNaviCloseBtnTouched:process Navi closed successfully\n");
    // }
    //else
    //{
    //    addPlainText("onNaviCloseBtnTouched:process Navi is already closed\n");
    // }
    /****20150202(begin)*******/
    HWND hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
    if (hwndNavigator != NULL)
    {
        COPYDATASTRUCT struCopyData;
        struCopyData.dwData = WM_STOP_LOG;
        ::SendMessage(hwndNavigator, WM_COPYDATA, (WPARAM)hwndNavigator, (LPARAM)&struCopyData);
        Sleep(500);
        ::SendMessage(hwndNavigator, WM_SYSCOMMAND, SC_CLOSE, 0);
    }
    else
        addPlainText("onNaviCloseBtnTouched()Mobile Navigator has closed.\n");
    /****20150202(end)******/
    /****20150204(begin)********/
    Sleep(2000);
    switchButtonInGroup(ui->NaviOpenBtn, ui->NaviCloseBtn);
    /*******20150227(begin)***********/
    if (ui->LaveOpenBtn->isEnabled())
    {
        disableButton(ui->logOpenBtn);
        /***20150303(begin)********/
        disableButton(ui->logCloseBtn);
        /***20150303(end)********/
    }
    /*******20150227(end)*************/
    if (NULL != m_pProcessNavi)
    {
        m_pProcessNavi->close();
        SAFE_DELETE(m_pProcessNavi);
        addPlainText("onNaviCloseBtnTouched:process Navi closed successfully\n");
    }
    else
        addPlainText("onNaviCloseBtnTouched:process Navi is already closed\n");
    /****20150204(end)********/
    if (NULL != m_pProcessCameraCan)
    {
        m_pProcessCameraCan->close();
        SAFE_DELETE(m_pProcessCameraCan);
        addPlainText("onNaviCloseBtnTouched:process CameraCan closed successfully\n");
    }
    else
        addPlainText("onNaviCloseBtnTouched:process CameraCan is already closed\n");
    /****20150211(begin)***********/
    navi_status = NAVI_CLOSED;
    /****20150211(end)************/
}

void ControlPage::onLaveOpenBtnTouched(void)
{
    HANDLE hCameraProcess = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("CAM_CAPTURE_EXCLUSIVE_EVENT"));
    if (NULL != hCameraProcess)
    {
        addPlainText("onLaveOpenBtnTouched Camera process already exists.\n");
        ::CloseHandle(hCameraProcess);
        hCameraProcess = NULL;
        return;
    }
    disableButtonInGroup(ui->LaveCloseBtn, ui->LaveOpenBtn);
    addPlainText("onLaveOpenBtnTouched waiting for open result......\n");
    emit sigSendStartCC();
}

void ControlPage::onLaveOpenBtnSucceeded(void)
{
    switchButtonInGroup(ui->LaveCloseBtn, ui->LaveOpenBtn);
    /******20150227(begin)***********/
    enableButton(ui->logOpenBtn);
    ui->imageCheckBox->setEnabled(TRUE);
    /******20150227(end)************/
    if (NULL == showLaveOneScreen)
    {
        showLaveOneScreen = new ShowScreen();
        showLaveOneScreen->setWindowTitle("Sign");
        showLaveOneScreen->setImageScale(m_nScaleW >= m_nScaleH ? m_nScaleH : m_nScaleW);
        showLaveOneScreen->setBaseSize((int)(m_nScaleW * m_nSignWidth), (int)(m_nScaleH * m_nSignHeight));
        showLaveOneScreen->setFixedSize((int)(m_nScaleW * m_nSignWidth), (int)(m_nScaleH * m_nSignHeight));
        connect(controlThreadSign, SIGNAL(notifyShowLaveOneScreen(uchar*, int, int)), showLaveOneScreen, SLOT(onShowScreen(uchar*, int, int)), Qt::BlockingQueuedConnection);
        showLaveOneScreen->move((int)(m_nScaleW * m_nNaviWidth), 0);
        showLaveOneScreen->show();
    }
    if (NULL == showLaveTwoScreen)
    {
        showLaveTwoScreen = new ShowScreen();
        showLaveTwoScreen->setWindowTitle("Lane");
        showLaveTwoScreen->setImageScale(m_nScaleW >= m_nScaleH ? m_nScaleH : m_nScaleW);
        showLaveTwoScreen->setBaseSize((int)(m_nScaleW * m_nLaneWidth), (int)(m_nScaleH * m_nLaneHeight));
        showLaveTwoScreen->setFixedSize((int)(m_nScaleW * m_nLaneWidth), (int)(m_nScaleH * m_nLaneHeight));
        connect(controlThreadLane, SIGNAL(notifyShowLaveTwoScreen(uchar*, int, int)), showLaveTwoScreen, SLOT(onShowScreen(uchar*, int, int)), Qt::BlockingQueuedConnection);
        showLaveTwoScreen->move(0, (int)(m_nScaleH * m_nNaviHeight));
        showLaveTwoScreen->show();
    }
    if (NULL == showRoadScreen)
    {
        showRoadScreen = new RoadScreen();
        showRoadScreen->setWindowTitle("Road");
        showRoadScreen->setBaseSize((int)(m_nScaleW * m_nRoadWidth), (int)(m_nScaleH * m_nRoadHeight));
        showRoadScreen->setFixedSize((int)(m_nScaleW * m_nRoadWidth), (int)(m_nScaleH * m_nRoadHeight));
        ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_LANENUMBER(WPARAM, LPARAM)), showRoadScreen, SLOT(onChangeRoadScreen(WPARAM, LPARAM)));
        showRoadScreen->move((int)(m_nScaleW * m_nNaviWidth), showLaveOneScreen->baseSize().height());
        showRoadScreen->show();
    }
    switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, TRUE);
    /****20150122(begin)*************/
    switchSliderAndSpinBoxInGroup(ui->expValueSlider, ui->expValueBox, TRUE);
    connect(controlThreadRecvCamNotify, SIGNAL(sigDisplayGrayValue(int)), this, SLOT(setGrayValueBox(int)));
    /****20150122(end)*************/
    connect(controlThreadRecvCamNotify, SIGNAL(sigDisplayRecogType(const QString&)), ui->trafficRecgTypeLabel, SLOT(setText(const QString&)));
    connect(controlThreadRecvCamNotify, SIGNAL(sigDisplayExposureTime(int)), this, SLOT(setExpValueBox(int)));
    //connect(getRootViewer(), SIGNAL(notifyCloseAll()), this, SLOT(onLaveCloseBtnTouched()));
    connect(getRootViewer(), SIGNAL(notifyCloseCam()), this, SLOT(onLaveCloseBtnTouched()));
    addPlainText("onLaveOpenBtnTouched open successfully.\n");
    b_isCamCapture = true;
    //    g_camControl.StartCamDebug();
}

void ControlPage::onLaveOpenBtnFailed(void)
{
    switchButtonInGroup(ui->LaveOpenBtn, ui->LaveCloseBtn);
    switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, FALSE);
    qDebug() << "onLaveOpenBtnTouched open failed.";
    addPlainText("onLaveOpenBtnTouched open failed.\n");
    b_isCamCapture = true;
}

void ControlPage::setGrayValueBox(int iValue)
{
    if (ui->grayValueBox != NULL)
    {
        if (1 == m_ucAcceptGrayValue)
        {
            ui->grayValueBox->setValue(iValue);
            qDebug() << "ControlPage::setGrayValueBox1" << iValue;
        }
        else
            qDebug() << "ControlPage::setGrayValueBox2" << iValue;
    }
}

/******20150122(begin)********/
void ControlPage::setExpValueBox(int iValue)
{
    if (ui->expValueBox != NULL)
    {
        if (1 == m_uAcceptExpValue)
        {
            ui->expValueBox->setValue(iValue);
            qDebug() << "ControlPage::setExpValueBox" << iValue;
        }
        else
            qDebug() << "do not accecpt expval from cam, slider is not released" << iValue;
    }
}
/******20150122(end)********/

void ControlPage::onLaveCloseBtnTouched(void)
{
    disableButtonInGroup(ui->LaveOpenBtn, ui->LaveCloseBtn);
    //switchButtonInGroup(ui->LaveOpenBtn, ui->LaveCloseBtn);
    switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, FALSE);
    addPlainText("onNLaveCloseBtnTouched:waiting for close result......\n");
    b_isCamCapture = false;
    emit sigSendEndCC();
}

void ControlPage::onLaveCloseBtnSucceeded(void)
{
    /****20150121 (begin)********/
    //disableButtonInGroup(ui->logBtnCam,ui->imageBtnCam);
    //ui->logBtnCam->setText("LOGON");
    //ui->imageBtnCam->setText("IMGON");
    //m_camTextlogFlag = CAMERA_TEXTLOG_ON;
    //m_camImagelogFlag = CAMERA_IMAGELOG_ON;
    /****20150121 (end)********/
    switchButtonInGroup(ui->LaveOpenBtn, ui->LaveCloseBtn);
    switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, FALSE);
    /******20150227(begin)***********/
    if (ui->NaviOpenBtn->isEnabled())
    {
        disableButton(ui->logOpenBtn);
        /*****20150303(begin)**************/
        disableButton(ui->logCloseBtn);
        /*****20150303(end)**************/
    }
    /*****20150303(begin)**************/
    if (ui->imageCheckBox->isChecked())
        ui->imageCheckBox->setCheckState(Qt::Unchecked);
    /*****20150303(end)**************/
    ui->imageCheckBox->setEnabled(FALSE);
    /******20150227(end)*************/
    if (NULL != showLaveOneScreen)
    {
        showLaveOneScreen->close();
        SAFE_DELETE(showLaveOneScreen);
    }
    if (NULL != showLaveTwoScreen)
    {
        showLaveTwoScreen->close();
        SAFE_DELETE(showLaveTwoScreen);
    }
    if (NULL != showRoadScreen)
    {
        showRoadScreen->close();
        SAFE_DELETE(showRoadScreen);
    }
    qDebug() << "onNLaveCloseBtnTouched:process closed successfully\n";
    addPlainText("onNLaveCloseBtnTouched:process closed successfully\n");
}

void ControlPage::onLaveCloseBtnFailed(void)
{
    switchButtonInGroup(ui->LaveCloseBtn, ui->LaveOpenBtn);
    switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, TRUE);
    qDebug() << "onLaveCloseBtnFailed close failed.";
    addPlainText("onLaveCloseBtnFailed close failed.\n");
}

void ControlPage::onLogOpenBtnTouched(void)
{
    /*******20150227(begin)***********/
    QString strPath1 = QCoreApplication::applicationFilePath();
    qDebug() << "Application Path:" << strPath1;
    QString runDirPath = QDir::currentPath();
    qDebug() << "Current Dir Path:" << runDirPath;
    QString logPath = "\\log\\";
    QString strCurrentDateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    g_logPath = runDirPath + logPath + strCurrentDateTime + "\\";
    /*****20150304(begin)*************/
    QDir logDir;
    if (!logDir.exists(g_logPath))
    {
        qDebug() << "onLogOpenBtnTouched : make log path:";
        logDir.mkpath(g_logPath);
    }
    /*****20150304(begin)*************/
    /*******20150227(end)***********/
    sendNaviLogStartCmd();   //??¡À¡ê¡ä?log¦Ì??¡¤??¡ä???Navi
    sendCamLogStartCmd();    //??¡À¡ê¡ä?log¦Ì??¡¤??¡ä???Camera
    if (ui->imageCheckBox->isChecked())
        sendCamImageStartCmd();
    switchButtonInGroup(ui->logCloseBtn, ui->logOpenBtn);
}

void ControlPage::onLogCloseBtnTouched(void)
{
    switchButtonInGroup(ui->logOpenBtn, ui->logCloseBtn);
    if (ui->imageCheckBox->isChecked())
    {
        ui->imageCheckBox->setCheckState(Qt::Unchecked);
        sendCamImageStopCmd();
    }
    sendNaviLogStopCmd();
    sendCamLogStopCmd();
}

/**********20150227(begin)*****************/
void ControlPage::handleCamImage(int check)
{
    if (ui->logCloseBtn->isEnabled())
    {
        if (ui->imageCheckBox->isChecked())  //¡À¡ê¡ä?image
        {
            //onLogCloseBtnTouched();
            //1?Log
            sendNaviLogStopCmd();
            sendCamLogStopCmd();
            switchButtonInGroup(ui->logOpenBtn, ui->logCloseBtn);
            //?aLog
            onLogOpenBtnTouched();
        }
        else //2?¡À¡ê¡ä?image
        {
            //onLogCloseBtnTouched();
            //1?image
            sendCamImageStopCmd();
            //1?log
            sendNaviLogStopCmd();
            sendCamLogStopCmd();
            switchButtonInGroup(ui->logOpenBtn, ui->logCloseBtn);
            //?aLog
            onLogOpenBtnTouched();
        }
    }
    else
        qDebug("handleCamImage  log on btn is not opened");
}
/**********20150227(end)*****************/

void ControlPage::sendCamImageStartCmd()
{
    HWND hwndCam = ::FindWindow(NULL, TEXT("Lane"));
    if (hwndCam != NULL)
    {
        qDebug("ControlPage::sendCamImageStartCmd() start to save camera image log.\n");
        ////ReqCamEnableSavePic(1);
        addPlainText("start to save camera image\n");
    }
    else
        addPlainText("Can't find Camera.\n");
}

void ControlPage::sendCamImageStopCmd()
{
    HWND hwndCam = ::FindWindow(NULL, TEXT("Lane"));
    if (hwndCam != NULL)
    {
        qDebug("ControlPage::sendCamImageStopCmd() end to save camera image log.\n");
        ////ReqCamEnableSavePic(0);
        addPlainText("stop to save camera image\n");
    }
    else
        addPlainText("Camera has closed.\n");
}

/******20150121(end)**********/

/*******20150113(begin)******/
void ControlPage::handleEnterPark()
{
    qDebug("HighWay : Leave\n");
    addPlainText("HighWay : Leave\n");
    ui->trafficRecgTypeLabel_5->setText("HighWay : Leave");
}

void ControlPage::handleLeavePark()
{
    qDebug("HighWay : Enter\n");
    addPlainText("HighWay : Enter\n");
    ui->trafficRecgTypeLabel_5->setText("HighWay : Enter");
}
/*******20150113(end)******/

void ControlPage::addPlainText(const QString& text)
{
    ui->messageShower->moveCursor(QTextCursor::End);
    ui->messageShower->insertPlainText(QTime::currentTime().toString() + "->:" + text);
    ui->messageShower->moveCursor(QTextCursor::End);
}

void ControlPage::getInfo()
{
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();
    g_nActScreenW = screenRect.width();
    g_nActScreenH = screenRect.height();
    //Read config file for displaying
    QFileInfo fConfigIniFileInfo(CONFIG_INI_FILE);
    if (fConfigIniFileInfo.exists())
    {
        QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
        m_nNaviWidth  = iniSettings.value("Navi/NaviWidth").toInt();
        m_nNaviHeight = iniSettings.value("Navi/NaviHeight").toInt();
        m_nLaneWidth =  iniSettings.value("Lane/LaneWidth").toInt();
        m_nLaneHeight = iniSettings.value("Lane/LaneHeight").toInt();
        m_nSignWidth =  iniSettings.value("Sign/SignWidth").toInt();
        m_nSignHeight = iniSettings.value("Sign/SignHeight").toInt();
        m_nRoadWidth =  iniSettings.value("Road/RoadWidth").toInt();
        m_nRoadHeight = iniSettings.value("Road/RoadHeight").toInt();
        m_nScreenWidth =  iniSettings.value("Screen/ScreenWidth").toInt();
        m_nScreenHeight = iniSettings.value("Screen/ScreenHeight").toInt();
    }
    else //use default value
    {
    }
    m_nScaleW = (float)(g_nActScreenW) / (float)(m_nScreenWidth);
    m_nScaleH = (float)(g_nActScreenH) / (float)(m_nScreenHeight);
}

void ControlPage::disableButtonInGroup(QPushButton* buttonDisabled1, QPushButton* buttonDisabled2)
{
    buttonDisabled1->setDisabled(TRUE);
    buttonDisabled1->setStyleSheet("QPushButton{color:gray;background:#153152}");
    buttonDisabled2->setDisabled(TRUE);
    buttonDisabled2->setStyleSheet("QPushButton{color:gray;background:#153152}");
}
/****add by runz(begin) 2015.01.12*********/
void ControlPage::enableButton(QPushButton* buttonEnable)
{
    buttonEnable->setEnabled(TRUE);
    buttonEnable->setStyleSheet("QPushButton{color:white;background:#335F90}");
}
void ControlPage::disableButton(QPushButton* buttonDisable)
{
    buttonDisable->setDisabled(TRUE);
    buttonDisable->setStyleSheet("QPushButton{color:gray;background:#153152}");
}
/****add by runz(begin) 2015.01.12*********/

void ControlPage::switchButtonInGroup(QPushButton* buttonEnabled, QPushButton* buttonDisabled)
{
    buttonEnabled->setStyleSheet("QPushButton{color:white;background:#335F90}");
    buttonDisabled->setDisabled(TRUE);
    buttonDisabled->setStyleSheet("QPushButton{color:gray;background:#153152}");
    buttonEnabled->setDisabled(FALSE);
}

void ControlPage::switchSliderAndSpinBoxInGroup(QSlider* valueSlider, QSpinBox* valueBox, BOOL bEnabled)
{
    valueSlider->setEnabled(bEnabled);
    valueBox->setEnabled(bEnabled);
}

void ControlPage::setGrayValue()
{
    if (CAM_STATUS_RUNNING != g_camControl.GetCamStatus())
    {
        qDebug("ControlPage::setGrayValue:Camera process is not running. CamStatus:%d",
               g_camControl.GetCamStatus());
        QMessageBox::information(NULL, "Warning", "Camera process is not running.");
    }
    else
    {
        qDebug("ControlPage::setGrayValue:%d", ui->grayValueSlider->value());
        ////ReqCamSetGrayValue(ui->grayValueSlider->value());
    }
}

/*****20150122 (begin)***********/
void ControlPage::setExpValue()
{
    if (CAM_STATUS_RUNNING != g_camControl.GetCamStatus())
    {
        qDebug("ControlPage::setExpsoureValue:Camera process is not running. CamStatus:%d",
               g_camControl.GetCamStatus());
        QMessageBox::information(NULL, "Warning", "Camera process is not running.");
    }
    else
    {
        qDebug("ControlPage::setExpsoureValue:%d", ui->expValueSlider->value());
        ////ReqCamExposureValue(ui->expValueSlider->value());
    }
}

void ControlPage::setExpValueAccept()
{
    m_uAcceptExpValue = 1;
    qDebug("ControlPage::setExpValueAccept() is called");
}

void ControlPage::setExpValueIgnore()
{
    m_uAcceptExpValue = 0;
    qDebug("ControlPage::setGrayValueIgnore() is called");
}

void ControlPage::onReqAddExpValue()
{
    qDebug("ControlPage::onReqAddExpValue enter");
    //if(b_isCamCapture)
    if (CAM_STATUS_RUNNING != g_camControl.GetCamStatus())
        addPlainText("onReqAddExpValue() CamCapture is't started.\n");
    else
    {
        int value = ui->expValueSlider->value();
        value = value + 1 ;
        ////ReqCamExposureValue(value);
        qDebug("ControlPage::onReqAddExpValue:%d", value);
    }
}

void ControlPage::onReqPlusExpValue()
{
    qDebug("ControlPage::onReqPlusExpValue enter");
    //if(b_isCamCapture)
    if (CAM_STATUS_RUNNING != g_camControl.GetCamStatus())
        addPlainText("onReqPlusExpValue() CamCapture is't started.\n");
    else
    {
        int value = ui->expValueSlider->value();
        value = value - 1;
        if (value >= 0)
        {
            ////ReqCamExposureValue(value);
            qDebug("ControlPage::onReqPlusExpValue:%d", value);
        }
        else
            qDebug("ControlPage::onReqPlusExpValue <0");
    }
}

/*****20150122 (end)**********/


void ControlPage::onReqCamEnableAutoExposure()
{
    //if(b_isCamCapture)
    if (CAM_STATUS_RUNNING != g_camControl.GetCamStatus())
        addPlainText("onReqCamEnableAutoExposure() CamCapture is't started.\n");
    else
    {
        addPlainText("onReqCamEnableAutoExposure is called.\n");
        ////int iRet = ReqCamEnableAutoExposure(1);
        /****20150122(begin)***********/
        switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, TRUE);
        /****20150122(end)***********/
        ui->autoExposureLabel->setText("ON");
    }
}

void ControlPage::onReqCamDisableAutoExposure()
{
    //if(b_isCamCapture)
    if (CAM_STATUS_RUNNING != g_camControl.GetCamStatus())
        addPlainText("onReqCamDisableAutoExposure() CamCapture is't started.\n");
    else
    {
        addPlainText("onReqCamDisableAutoExposure is called.\n");
        ////ReqCamEnableAutoExposure(0);
        /****20150122(begin)***********/
        switchSliderAndSpinBoxInGroup(ui->grayValueSlider, ui->grayValueBox, FALSE);
        /****20150122(end)***********/
        ui->autoExposureLabel->setText("OFF");
    }
}

/******show time min , second, ms 20140120(begin)********/
void ControlPage::showTime()
{
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm:ss:zzz");
    ui->lcdNumber->display(text);
}
/******show time min , second, ms 20140120(end)********/



