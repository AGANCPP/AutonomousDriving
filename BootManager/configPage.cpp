
#include "configPage.h"
#include "BaseDefine.h"
#include "systemlog.h"
#include "controlthread.h"
#include "ui_configPage.h"

extern ControlThreadRecvCamNotify* g_controlThreadRecvCamNotify;
//extern MyApplication* g_strMyApplication;

ConfigPage::ConfigPage(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::ConfigPage)
{
    ui->setupUi(this);
    init();
}

ConfigPage::~ConfigPage()
{
    qDebug("~ConfigPage");
    delete ui;
}

void ConfigPage::init()
{
    nRegionCountry = CONFIG_DEF_REGION_COUNTRY;
    nRegionArea = CONFIG_DEF_REGION_AREA;
    QFileInfo fConfigIniFileInfo;
    fConfigIniFileInfo.setFile(CONFIG_INI_FILE);
    if (fConfigIniFileInfo.exists())
    {
        qDebug() << "config file exists, use config file value:";
        QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
        nRegionCountry  = iniSettings.value("Region/Country").toInt();
        nRegionArea  = iniSettings.value("Region/Area").toInt();
    }
    else
        qWarning() << "config file not exist, use default value.";
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
    }
    else
    {
        ui->NaviMapSet1Btn->setIcon(QIcon("./image/icon_whhj.png"));
        ui->NaviMapSet2Btn->setIcon(QIcon("./image/icon_whmy.png"));
        ui->NaviMapSet3Btn->setVisible(false);
        ui->NaviMapSet4Btn->setVisible(false);
        ui->NaviMapSet5Btn->setVisible(false);
        ui->NaviMapSet6Btn->setVisible(false);
    }
    strProcessBatArg[CONFIG_REGION_COUNTRY_JPN][CONFIG_REGION_AREA_JPN_00] = "QYD_1";
    strProcessBatArg[CONFIG_REGION_COUNTRY_JPN][CONFIG_REGION_AREA_JPN_01] = "QYD_2";
    strProcessBatArg[CONFIG_REGION_COUNTRY_JPN][CONFIG_REGION_AREA_JPN_02] = "SDG";
    strProcessBatArg[CONFIG_REGION_COUNTRY_JPN][CONFIG_REGION_AREA_JPN_03] = "BSG";
    strProcessBatArg[CONFIG_REGION_COUNTRY_JPN][CONFIG_REGION_AREA_JPN_04] = "3rdJB";
    strProcessBatArg[CONFIG_REGION_COUNTRY_JPN][CONFIG_REGION_AREA_JPN_05] = "OPG";
    strProcessBatArg[CONFIG_REGION_COUNTRY_CHN][CONFIG_REGION_AREA_CHN_00] = "JJXY";
    strProcessBatArg[CONFIG_REGION_COUNTRY_CHN][CONFIG_REGION_AREA_CHN_01] = "JJXY";
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
        QWidget::connect(ui->NaviMapSet1Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet1BtnTouched()));
        QWidget::connect(ui->NaviMapSet2Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet2BtnTouched()));
        QWidget::connect(ui->NaviMapSet3Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet3BtnTouched()));
        QWidget::connect(ui->NaviMapSet4Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet4BtnTouched()));
        QWidget::connect(ui->NaviMapSet5Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet5BtnTouched()));
        QWidget::connect(ui->NaviMapSet6Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet6BtnTouched()));
    }
    else
    {
        QWidget::connect(ui->NaviMapSet1Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet1BtnTouched()));
        QWidget::connect(ui->NaviMapSet2Btn, SIGNAL(clicked()), this, SLOT(onNaviMapSet2BtnTouched()));
    }
}

void ConfigPage::onNaviMapSet1BtnTouched(void)
{
    qDebug() << "onNaviMapSet1BtnTouched";
    addPlainText("onNaviMapSet1BtnTouched");
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
        nRegionArea = CONFIG_REGION_AREA_JPN_00;   //QYD_1
    }
    else
        nRegionArea = CONFIG_REGION_AREA_CHN_00;
    naviMapSet();
}

void ConfigPage::onNaviMapSet2BtnTouched(void)
{
    qDebug() << "onNaviMapSet2BtnTouched";
    addPlainText("onNaviMapSet2BtnTouched");
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
        nRegionArea = CONFIG_REGION_AREA_JPN_01;   //QYD_2
    }
    else
        nRegionArea = CONFIG_REGION_AREA_CHN_01;
    naviMapSet();
}

void ConfigPage::onNaviMapSet3BtnTouched(void)
{
    qDebug() << "onNaviMapSet3BtnTouched";
    addPlainText("onNaviMapSet3BtnTouched");
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
        nRegionArea = CONFIG_REGION_AREA_JPN_02;  //SDG
        naviMapSet();
    }
    else
    {
        //        nRegionArea = CONFIG_REGION_AREA_CHN_02;
    }
}

void ConfigPage::onNaviMapSet4BtnTouched(void)
{
    qDebug() << "onNaviMapSet4BtnTouched";
    addPlainText("onNaviMapSet4BtnTouched");
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
        nRegionArea = CONFIG_REGION_AREA_JPN_03;  //BSG
        naviMapSet();
    }
    else
    {
        //        nRegionArea = CONFIG_REGION_AREA_CHN_02;
    }
}

void ConfigPage::onNaviMapSet5BtnTouched(void)
{
    qDebug() << "onNaviMapSet5BtnTouched";
    addPlainText("onNaviMapSet5BtnTouched");
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
        nRegionArea = CONFIG_REGION_AREA_JPN_04;      //3rdJB
        naviMapSet();
    }
    else
    {
        //        nRegionArea = CONFIG_REGION_AREA_CHN_02;
    }
}

void ConfigPage::onNaviMapSet6BtnTouched(void)
{
    qDebug() << "onNaviMapSet6BtnTouched";
    addPlainText("onNaviMapSet6BtnTouched");
    if (CONFIG_REGION_COUNTRY_JPN == nRegionCountry)
    {
        nRegionArea = CONFIG_REGION_AREA_JPN_05;      //OPG
        naviMapSet();
    }
    else
    {
        //        nRegionArea = CONFIG_REGION_AREA_CHN_02;
    }
}

void ConfigPage::naviMapSet(void)
{
    QProcess* procSetNaviMapBat;
    QString process_path = "ChangeData.bat";
    QString process_arg;
    QString strDebug;
    QFileInfo qFileInfo_Navi;
    if (!naviMapSetFileCheck())
        return;
    process_arg = strProcessBatArg[nRegionCountry][nRegionArea];
    qFileInfo_Navi.setFile(process_path);
    if (qFileInfo_Navi.exists())
    {
        procSetNaviMapBat = new QProcess(this);
        procSetNaviMapBat->start("cmd.exe", QStringList() << "/c" <<  process_path << process_arg);
        qDebug() << "cmd.exe" << " /c " <<  process_path << " " << process_arg;
        if (procSetNaviMapBat->waitForStarted())
        {
            procSetNaviMapBat->waitForFinished();
            strDebug = "Bat executed successfully.";
        }
        else
            strDebug = "Failed to start bat";
        qDebug() << strDebug;
        strDebug = "onNaviMapSetBtnTouched: " + strDebug + "\n";
        addPlainText(strDebug);
        SAFE_DELETE(procSetNaviMapBat);
    }
    else
    {
        qCritical("onNaviMapSetBtnTouched: ChangeData.bat file does not exist. Please copy one.");
        addPlainText("onNaviMapSetBtnTouched: ChangeData.bat file does not exist. Please copy one.\n");
        return;
    }
}

bool ConfigPage::naviMapSetFileCheck(void)
{
    QString setFilePath;
    setFilePath = "Navigator/bat/" + strProcessBatArg[nRegionCountry][nRegionArea] + "/mapdataioconfig.ini";
    QFileInfo fConfigIniFileInfo(setFilePath);
    if (fConfigIniFileInfo.exists())
        return true;
    else
    {
        qDebug() << "File not exist. " << setFilePath;
        addPlainText("File not exist.\n" + setFilePath);
        return false;
    }
}

void ConfigPage::addPlainText(const QString& text)
{
    //    m_MsgDisp1->moveCursor(QTextCursor::End);
    ui->txtMsgDisp->setText(QTime::currentTime().toString() + "->:" + text);
    //    m_MsgDisp1->moveCursor(QTextCursor::End);
}

void ConfigPage::getInfo()
{
}

