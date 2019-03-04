
#include "radarPage.h"
#include "rootviewer.h"
//#include "IpcAccess.h"
#include "BaseDefine.h"
#include "safeDelete.h"
#include "systemlog.h"

RadarPage::RadarPage(QWidget* parent)
    : QWidget(parent)
      ,messageShower(NULL),
      NaviOpenBtn(NULL),
      NaviCloseBtn(NULL),
      LaveOpenBtn(NULL),
      LaveCloseBtn(NULL),
      logOpenBtn(NULL),
      logCloseBtn(NULL),
      leftDistanceEditer(NULL),
      rightDistanceEditer(NULL),
      distance_XEditer(NULL),
      distance_YEditer(NULL),
      controlThreadSign(NULL),
      controlThreadLane(NULL),
      //showNaviScreen(NULL),
      showLaveOneScreen(NULL),
      showLaveTwoScreen(NULL),
      showRoadScreen(NULL),

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
      g_nActScreenH(0)
{
    init();
}

RadarPage::~RadarPage()
{
    qDebug("~RadarPage");
    SAFE_DELETE(messageShower);
    SAFE_DELETE(NaviOpenBtn);
    SAFE_DELETE(NaviCloseBtn);
    SAFE_DELETE(LaveOpenBtn);
    SAFE_DELETE(LaveCloseBtn);
    SAFE_DELETE(logOpenBtn);
    SAFE_DELETE(logCloseBtn);
    SAFE_DELETE(messageShower);
    SAFE_DELETE(leftDistanceEditer);
    SAFE_DELETE(rightDistanceEditer);
    SAFE_DELETE(distance_XEditer);
    SAFE_DELETE(distance_YEditer);
    SAFE_DELETE(controlThreadSign);
    SAFE_DELETE(controlThreadLane);
    //SAFE_DELETE(showNaviScreen);
    SAFE_DELETE(showLaveOneScreen);
    SAFE_DELETE(showLaveTwoScreen);
    SAFE_DELETE(showRoadScreen);
    if (NULL != m_pProcessCameraCan)
    {
        m_pProcessCameraCan->close();
        m_pProcessCameraCan = NULL;
    }
    if (NULL != m_pProcessNavi)
    {
        m_pProcessNavi->close();
        m_pProcessNavi = NULL;
    }
}

void RadarPage::init()
{
    m_pProcessCameraCan = NULL;
    m_pProcessNavi = NULL;
    m_pProcessVechileControl = NULL;
    // Radar
    QGroupBox* naviGroup = new QGroupBox(tr("Radar"));
    NaviOpenBtn = new QPushButton(tr("OPEN"));
    NaviOpenBtn->setMinimumHeight(20);
    NaviCloseBtn = new QPushButton(tr("CLOSE"));
    NaviCloseBtn->setMinimumHeight(20);
    NaviOpenBtn->setStyleSheet("QPushButton{background:#335F90}");
    NaviCloseBtn->setStyleSheet("QPushButton{background:#335F90}");
    QHBoxLayout* naviBtnLayout = new QHBoxLayout;
    naviBtnLayout->addWidget(NaviOpenBtn);
    naviBtnLayout->addWidget(NaviCloseBtn);
    QVBoxLayout* naviLayout = new QVBoxLayout;
    naviLayout->addLayout(naviBtnLayout);
    naviGroup->setLayout(naviLayout);
    naviGroup->setStyleSheet("QGroupBox{color:white;background:#153152}");
    // Vehicle Control
    QGroupBox* laveGroup = new QGroupBox(tr("Vehicle Control"));
    LaveOpenBtn = new QPushButton(tr("Steering"));
    LaveOpenBtn->setMinimumHeight(20);
    LaveCloseBtn = new QPushButton(tr("Brake"));
    LaveCloseBtn->setMinimumHeight(20);
    LaveOpenBtn->setStyleSheet("QPushButton{background:#335F90}");
    LaveCloseBtn->setStyleSheet("QPushButton{background:#335F90}");
    QHBoxLayout* laveBtnLayout = new QHBoxLayout;
    laveBtnLayout->addWidget(LaveOpenBtn);
    laveBtnLayout->addWidget(LaveCloseBtn);
    QVBoxLayout* laveLayout = new QVBoxLayout;
    laveLayout->addLayout(laveBtnLayout);
    laveGroup->setLayout(laveLayout);
    laveGroup->setStyleSheet("QGroupBox{color:white;background:#153152}");
    // Log Switch
    QGroupBox* logGroup = new QGroupBox(tr("LOG SWITCH"));
    logOpenBtn = new QPushButton(tr("OPEN"));
    logOpenBtn->setMinimumHeight(20);
    logCloseBtn = new QPushButton(tr("CLOSE"));
    logCloseBtn->setMinimumHeight(20);
    logOpenBtn->setStyleSheet("QPushButton{background:#335F90}");
    logCloseBtn->setStyleSheet("QPushButton{background:#335F90}");
    QHBoxLayout* logBtnLayout = new QHBoxLayout;
    logBtnLayout->addWidget(logOpenBtn);
    logBtnLayout->addWidget(logCloseBtn);
    QVBoxLayout* logSwitcgLayout = new QVBoxLayout;
    logSwitcgLayout->addLayout(logBtnLayout);
    logGroup->setLayout(logSwitcgLayout);
    logGroup->setStyleSheet("QGroupBox{color:white;background:#153152}");
    // Lave log
    QGroupBox* laveLogGroup = new QGroupBox(tr("Lave Event"));
    QLabel* leftDistance = new QLabel(tr("left Lane Distance:  "));
    leftDistance->setStyleSheet("QLabel{color:white}");
    leftDistanceEditer = new QLineEdit("0");
    leftDistanceEditer->setMinimumHeight(15);
    //    leftDistanceEditer->setMaximumWidth(50);
    leftDistanceEditer->setMinimumWidth(25);
    QHBoxLayout* leftLayout = new QHBoxLayout;
    leftLayout->addWidget(leftDistance);
    leftLayout->addWidget(leftDistanceEditer);
    QLabel* rightDistance = new QLabel(tr("Right Lane Distance: "));
    rightDistance->setStyleSheet("QLabel{color:white}");
    rightDistanceEditer = new QLineEdit("0");
    rightDistanceEditer->setMinimumHeight(15);
    //    rightDistanceEditer->setMaximumWidth(50);
    rightDistanceEditer->setMinimumWidth(25);
    QHBoxLayout* rightLayout = new QHBoxLayout;
    rightLayout->addWidget(rightDistance);
    rightLayout->addWidget(rightDistanceEditer);
    QVBoxLayout* laveDistanceLayout = new QVBoxLayout;
    laveDistanceLayout->addLayout(leftLayout);
    laveDistanceLayout->addLayout(rightLayout);
    QVBoxLayout* laveLogLayout = new QVBoxLayout;
    laveLogLayout->addLayout(laveDistanceLayout);
    laveLogGroup->setLayout(laveLogLayout);
    laveLogGroup->setStyleSheet("QGroupBox{color:white;background:#153152}");
    // Traffic log
    QGroupBox* TrafficLogGroup = new QGroupBox(tr("Traffic Event"));
    QLabel* distance_X = new QLabel(tr("Dis : X :  "));
    distance_X->setStyleSheet("QLabel{color:white}");
    distance_XEditer = new QLineEdit("0");
    distance_XEditer->setMinimumHeight(15);
    //    distance_XEditer->setMaximumWidth(50);
    distance_XEditer->setMinimumWidth(25);
    QHBoxLayout* XLayout = new QHBoxLayout;
    XLayout->addWidget(distance_X);
    XLayout->addWidget(distance_XEditer);
    QLabel* distance_Y = new QLabel(tr("Dis : Y :  "));
    distance_Y->setStyleSheet("QLabel{color:white}");
    distance_YEditer = new QLineEdit("0");
    distance_YEditer->setMinimumHeight(15);
    //    distance_YEditer->setMaximumWidth(50);
    distance_YEditer->setMinimumWidth(25);
    QHBoxLayout* YLayout = new QHBoxLayout;
    YLayout->addWidget(distance_Y);
    YLayout->addWidget(distance_YEditer);
    QVBoxLayout* TrafficLayout = new QVBoxLayout;
    TrafficLayout->addLayout(XLayout);
    TrafficLayout->addLayout(YLayout);
    QVBoxLayout* trafficLogLayout = new QVBoxLayout;
    trafficLogLayout->addLayout(TrafficLayout);
    TrafficLogGroup->setLayout(trafficLogLayout);
    TrafficLogGroup->setStyleSheet("QGroupBox{color:white;background:#153152}");
    QHBoxLayout* logLayout = new QHBoxLayout;
    logLayout->addWidget(laveLogGroup);
    logLayout->addWidget(TrafficLogGroup);
    messageShower = new QTextEdit("");
    messageShower->setMaximumHeight(50);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(naviGroup);
    mainLayout->addWidget(laveGroup);
    mainLayout->addWidget(logGroup);
    mainLayout->addLayout(logLayout);
    mainLayout->addWidget(messageShower);
    mainLayout->addStretch(1);
    mainLayout->setSpacing(2);
    setLayout(mainLayout);
    // Create thread
    controlThreadSign = new ControlThreadSign();
    controlThreadSign->start();
    controlThreadLane = new ControlThreadLane();
    controlThreadLane->start();
    // Get screen info(width/height)
    getInfo();
    // Connect signal and slot
    QWidget::connect(NaviOpenBtn, SIGNAL(clicked()), this, SLOT(onNaviOpenBtnTouched()));
    QWidget::connect(NaviCloseBtn, SIGNAL(clicked()), this, SLOT(onNaviCloseBtnTouched()));
    QWidget::connect(LaveOpenBtn, SIGNAL(clicked()), this, SLOT(onLaveOpenBtnTouched()));
    QWidget::connect(LaveCloseBtn, SIGNAL(clicked()), this, SLOT(onLaveCloseBtnTouched()));
    QWidget::connect(logOpenBtn, SIGNAL(clicked()), this, SLOT(onLogOpenBtnTouched()));
    QWidget::connect(logCloseBtn, SIGNAL(clicked()), this, SLOT(onLogCloseBtnTouched()));
    
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_TRAFFICSIGNAL_XYZ_L(const QString&)), leftDistanceEditer, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_TRAFFICSIGNAL_XYZ_R(const QString&)), rightDistanceEditer, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_LANEDISTANCE_LD_RD_X(const QString&)), distance_XEditer, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_LANEDISTANCE_LD_RD_Y(const QString&)), distance_YEditer, SLOT(setText(const QString&)));
}

void RadarPage::onNaviOpenBtnTouched(void)
{
    if (NULL == m_pProcessVechileControl)
    {
        QString process_path = "VehicleControl/VehicleControl.exe";
        QFileInfo qFileInfo_VehicleControl;
        qFileInfo_VehicleControl.setFile(process_path);
        if (qFileInfo_VehicleControl.exists())
        {
            m_pProcessVechileControl = new QProcess(this);
            m_pProcessVechileControl->start(process_path);
            addPlainText("onNaviOpenBtnTouched: open VehicleControl process\n");
        }
        else
        {
            qCritical("onNaviOpenBtnTouched: VehicleControl process file does not exist. Please copy one.");
            addPlainText("onNaviOpenBtnTouched: VehicleControl process file does not exist. Please copy one.\n");
            return;
        }
    }
    else
        addPlainText("onNaviOpenBtnTouched: VehicleControl process is already opened\n");
}

void RadarPage::onNaviCloseBtnTouched(void)
{
    if (NULL != m_pProcessVechileControl)
    {
        m_pProcessVechileControl->close();
        m_pProcessVechileControl = NULL;
        addPlainText("onNaviCloseBtnTouched:process VehicleControl closed successfully\n");
    }
    else
        addPlainText("onNaviCloseBtnTouched:process VehicleControl is already closed\n");
}

void RadarPage::onLaveOpenBtnTouched(void)
{
    if (NULL != m_pProcessVechileControl)
    {
        QString direction = leftDistanceEditer->text();
        QString speed = rightDistanceEditer->text();
        ////ReqVehTurnWheel(direction.toInt(), speed.toInt());
        QString strLog;
        strLog = "ReqVehTurnWheel, direction:";
        strLog += direction;
        strLog += ";speed:";
        strLog += speed;
        strLog += "\n";
        addPlainText(strLog);
    }
    else
        addPlainText(" VehicleControl process is not opened\n");
}

void RadarPage::onLaveCloseBtnTouched(void)
{
    if (NULL != m_pProcessVechileControl)
    {
        QString strength = distance_XEditer->text();
        ////ReqVehBrake(strength.toInt());
        QString strLog;
        strLog = "ReqVehBrake, Strength:";
        strLog += strength;
        strLog += "\n";
        addPlainText(strLog);
    }
    else
        addPlainText(" VehicleControl process is not opened\n");
}

void RadarPage::onLogOpenBtnTouched(void)
{
    return;
    QDir dir;
    QString runDirPath = dir.currentPath();
    //g_strMobileNavigatorLogPath = runDirPath + "/log/";
    //qDebug()<<"str_len:"<<g_strMobileNavigatorLogPath.length();
    HWND hwndNavigator = NULL;
    hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
    COPYDATASTRUCT struCopyData;
    struCopyData.dwData = WM_LOG_PATH;
    //struCopyData.lpData = (void *)g_strMobileNavigatorLogPath.toStdWString().c_str();
    //struCopyData.cbData = g_strMobileNavigatorLogPath.length()*2;//wcslen(g_strMobileNavigatorLogPath)*sizeof(kn_char);
    qDebug() << "struCopyData.dwData:" << struCopyData.dwData
             << "; struCopyData.lpData:" << struCopyData.lpData
             << "; struCopyData.cbData:" << struCopyData.cbData;
    ::SendMessage(hwndNavigator, WM_COPYDATA, (WPARAM)hwndNavigator, (LPARAM)&struCopyData);
    struCopyData.dwData = WM_START_LOG;
    ::SendMessage(hwndNavigator, WM_COPYDATA, (WPARAM)hwndNavigator, (LPARAM)&struCopyData);
    addPlainText("onLogOpenBtnTouched\n");
}

void RadarPage::onLogCloseBtnTouched(void)
{
    return;
    HWND hwndNavigator = ::FindWindow(NULL, TEXT("MobileNavigator"));
    COPYDATASTRUCT struCopyData;
    struCopyData.dwData = WM_STOP_LOG;
    ::SendMessage(hwndNavigator, WM_COPYDATA, (WPARAM)hwndNavigator, (LPARAM)&struCopyData);
    addPlainText("onLogCloseBtnTouched\n");
}

void RadarPage::addPlainText(const QString& text)
{
    messageShower->moveCursor(QTextCursor::End);
    messageShower->insertPlainText(QTime::currentTime().toString() + "->:" + text);
    messageShower->moveCursor(QTextCursor::End);
}

void RadarPage::getInfo()
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

