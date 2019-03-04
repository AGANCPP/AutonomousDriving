

#include "aboutPage.h"
//#include "rootviewer.h"
//#include "IpcAccess.h"
#include "BaseDefine.h"
//#include "safeDelete.h"
#include "systemlog.h"

#include "controlthread.h"
#include "ui_aboutPage.h"

extern ControlThreadRecvCamNotify* g_controlThreadRecvCamNotify;
////extern MyApplication* g_strMyApplication;

AboutPage::AboutPage(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::AboutPage)
      //      messageShower(NULL),
      //      NaviOpenBtn(NULL),
      //      NaviCloseBtn(NULL),
      //      LaveOpenBtn(NULL),
      //      LaveCloseBtn(NULL),
      //      logOpenBtn(NULL),
      //      logCloseBtn(NULL),
      //      leftDistanceEditer(NULL),
      //      rightDistanceEditer(NULL),
      //      distance_XEditer(NULL),
      //      distance_YEditer(NULL),
      //      controlThreadSign(NULL),
      //      controlThreadLane(NULL),
      //      //showNaviScreen(NULL),
      //      showLaveOneScreen(NULL),
      //      showLaveTwoScreen(NULL),
      //      showRoadScreen(NULL),

      //      m_nNaviWidth(CONFIG_DEF_NAVI_WIDTH),
      //      m_nNaviHeight(CONFIG_DEF_NAVI_HEIGHT),
      //      m_nLaneWidth(CONFIG_DEF_LANE_WIDTH),
      //      m_nLaneHeight(CONFIG_DEF_LANE_HEIGHT),
      //      m_nSignWidth(CONFIG_DEF_SIGN_WIDTH),
      //      m_nSignHeight(CONFIG_DEF_SIGN_HEIGHT),
      //      m_nRoadWidth(CONFIG_DEF_ROAD_WIDTH),
      //      m_nRoadHeight(CONFIG_DEF_ROAD_HEIGHT),
      //      m_nScreenWidth(CONFIG_DEF_SCR_WIDTH),
      //      m_nScreenHeight(CONFIG_DEF_SCR_HEIGHT),

      //      g_nActScreenW(0),
      //      g_nActScreenH(0)
{
    ui->setupUi(this);
    init();
}

AboutPage::~AboutPage()
{
    qDebug("~AboutPage");
    delete ui;
    //    SAFE_DELETE(messageShower);
    //    SAFE_DELETE(NaviOpenBtn);
    //    SAFE_DELETE(NaviCloseBtn);
    //    SAFE_DELETE(LaveOpenBtn);
    //    SAFE_DELETE(LaveCloseBtn);
    //    SAFE_DELETE(logOpenBtn);
    //    SAFE_DELETE(logCloseBtn);
    //    SAFE_DELETE(messageShower);
    //    SAFE_DELETE(leftDistanceEditer);
    //    SAFE_DELETE(rightDistanceEditer);
    //    SAFE_DELETE(distance_XEditer);
    //    SAFE_DELETE(distance_YEditer);
    //    SAFE_DELETE(controlThreadSign);
    //    SAFE_DELETE(controlThreadLane);
    //    //SAFE_DELETE(showNaviScreen);
    //    SAFE_DELETE(showLaveOneScreen);
    //    SAFE_DELETE(showLaveTwoScreen);
    //    SAFE_DELETE(showRoadScreen);
    //    if(NULL != m_pProcessCameraCan)
    //    {
    //        m_pProcessCameraCan->close();
    //        m_pProcessCameraCan = NULL;
    //    }
    //    if(NULL != m_pProcessNavi)
    //    {
    //        m_pProcessNavi->close();
    //        m_pProcessNavi = NULL;
    //    }
}

void AboutPage::init()
{
    double version = 0.0;
    QFileInfo fConfigIniFileInfo;
    fConfigIniFileInfo.setFile(CONFIG_INI_FILE);
    if (fConfigIniFileInfo.exists())
    {
        qDebug() << "config file exists, use config file value:";
        QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
        version  = iniSettings.value("About/Version").toDouble();
        ui->label_2->setNum(version);
    }
    else //use default value
        qWarning() << "config file not exist, use default value.";
    connect(g_controlThreadRecvCamNotify, SIGNAL(sigDisplayCamVersion(const QString&)), ui->CamCaptureVerison, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgMobileNavigator_VERSION(const QString&)), ui->NavigationVersion, SLOT(setText(const QString&)));
    ////connect(g_strMyApplication, SIGNAL(MsgCameraCAN_VERSION(const QString&)), ui->CameraCanVerision, SLOT(setText(const QString&)));
}

void AboutPage::onNaviOpenBtnTouched(void)
{
    //    if(NULL == m_pProcessVechileControl)
    //    {
    //        QString process_path = "VehicleControl/VehicleControl.exe";
    //        QFileInfo qFileInfo_VehicleControl;
    //        qFileInfo_VehicleControl.setFile(process_path);
    //        if(qFileInfo_VehicleControl.exists())
    //        {
    //            m_pProcessVechileControl = new QProcess(this);
    //            m_pProcessVechileControl->start(process_path);
    //            addPlainText("onNaviOpenBtnTouched: open VehicleControl process\n");
    //        }
    //        else
    //        {
    //            qCritical("onNaviOpenBtnTouched: VehicleControl process file does not exist. Please copy one.");
    //            addPlainText("onNaviOpenBtnTouched: VehicleControl process file does not exist. Please copy one.\n");
    //            return;
    //        }
    //    }
    //    else
    //    {
    //        addPlainText("onNaviOpenBtnTouched: VehicleControl process is already opened\n");
    //    }
}

void AboutPage::onNaviCloseBtnTouched(void)
{
    //    if(NULL != m_pProcessVechileControl)
    //    {
    //        m_pProcessVechileControl->close();
    //        m_pProcessVechileControl = NULL;
    //        addPlainText("onNaviCloseBtnTouched:process VehicleControl closed successfully\n");
    //    }
    //    else
    //    {
    //        addPlainText("onNaviCloseBtnTouched:process VehicleControl is already closed\n");
    //    }
}

void AboutPage::onLaveOpenBtnTouched(void)
{
    //    if(NULL != m_pProcessVechileControl)
    //    {
    //        QString direction = leftDistanceEditer->text();
    //        QString speed = rightDistanceEditer->text();
    ////        ReqVehTurnWheel(direction.toInt(), speed.toInt());
    //        QString strLog;
    //        strLog = "ReqVehTurnWheel, direction:";
    //        strLog += direction;
    //        strLog += ";speed:";
    //        strLog += speed;
    //        strLog += "\n";
    //        addPlainText(strLog);
    //    }
    //    else
    //    {
    //        addPlainText(" VehicleControl process is not opened\n");
    //    }
}

void AboutPage::onLaveCloseBtnTouched(void)
{
    //    if(NULL != m_pProcessVechileControl)
    //    {
    //        QString strength = distance_XEditer->text();
    ////        ReqVehBrake(strength.toInt());
    //        QString strLog;
    //        strLog = "ReqVehBrake, Strength:";
    //        strLog += strength;
    //        strLog += "\n";
    //        addPlainText(strLog);
    //    }
    //    else
    //    {
    //        addPlainText(" VehicleControl process is not opened\n");
    //    }
}

void AboutPage::onLogOpenBtnTouched(void)
{
    //    return;
    //    QDir dir;
    //    QString runDirPath=dir.currentPath();
    ////    g_strMobileNavigatorLogPath = runDirPath + "/log/";
    ////    qDebug()<<"str_len:"<<g_strMobileNavigatorLogPath.length();
    //    HWND hwndNavigator = NULL;
    //    hwndNavigator = ::FindWindow(NULL,TEXT("MobileNavigator"));
    //    COPYDATASTRUCT struCopyData;
    ////    struCopyData.dwData = WM_LOG_PATH;
    ////    struCopyData.lpData = (void *)g_strMobileNavigatorLogPath.toStdWString().c_str();
    ////    struCopyData.cbData = g_strMobileNavigatorLogPath.length()*2;//wcslen(g_strMobileNavigatorLogPath)*sizeof(kn_char);
    //    qDebug() << "struCopyData.dwData:" << struCopyData.dwData
    //             << "; struCopyData.lpData:" << struCopyData.lpData
    //             << "; struCopyData.cbData:" << struCopyData.cbData;
    //    ::SendMessage(hwndNavigator,WM_COPYDATA,(WPARAM)hwndNavigator,(LPARAM)&struCopyData);
    ////    struCopyData.dwData = WM_START_LOG;
    //    ::SendMessage(hwndNavigator,WM_COPYDATA,(WPARAM)hwndNavigator,(LPARAM)&struCopyData);
    //    addPlainText("onLogOpenBtnTouched\n");
}

void AboutPage::onLogCloseBtnTouched(void)
{
    //    return;
    //    HWND hwndNavigator = ::FindWindow(NULL,TEXT("MobileNavigator"));
    //    COPYDATASTRUCT struCopyData;
    ////    struCopyData.dwData = WM_STOP_LOG;
    //    ::SendMessage(hwndNavigator,WM_COPYDATA,(WPARAM)hwndNavigator,(LPARAM)&struCopyData);
    //    addPlainText("onLogCloseBtnTouched\n");
}

void AboutPage::addPlainText(const QString& text)
{
    //    messageShower->moveCursor(QTextCursor::End);
    //    messageShower->insertPlainText(QTime::currentTime().toString()+"->:"+text);
    //    messageShower->moveCursor(QTextCursor::End);
}

void AboutPage::getInfo()
{
    //    QDesktopWidget* desktopWidget = QApplication::desktop();
    //    QRect screenRect = desktopWidget->screenGeometry();
    //    g_nActScreenW = screenRect.width();
    //    g_nActScreenH = screenRect.height();
    //Read config file for displaying
    //    QFileInfo fConfigIniFileInfo(CONFIG_INI_FILE);
    //    if(fConfigIniFileInfo.exists())
    {
        //        QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
        //        m_nNaviWidth  = iniSettings.value("Navi/NaviWidth").toInt();
        //        m_nNaviHeight = iniSettings.value("Navi/NaviHeight").toInt();
        //        m_nLaneWidth =  iniSettings.value("Lane/LaneWidth").toInt();
        //        m_nLaneHeight = iniSettings.value("Lane/LaneHeight").toInt();
        //        m_nSignWidth =  iniSettings.value("Sign/SignWidth").toInt();
        //        m_nSignHeight = iniSettings.value("Sign/SignHeight").toInt();
        //        m_nRoadWidth =  iniSettings.value("Road/RoadWidth").toInt();
        //        m_nRoadHeight = iniSettings.value("Road/RoadHeight").toInt();
        //        m_nScreenWidth =  iniSettings.value("Screen/ScreenWidth").toInt();
        //        m_nScreenHeight = iniSettings.value("Screen/ScreenHeight").toInt();
    }
    //    else //use default value
    //    {
    //    }
    //    m_nScaleW = (float)(g_nActScreenW)/(float)(m_nScreenWidth);
    //    m_nScaleH = (float)(g_nActScreenH)/(float)(m_nScreenHeight);
}

