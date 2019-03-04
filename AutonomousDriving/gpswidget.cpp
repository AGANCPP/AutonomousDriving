
#include <QtGui>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include "gpswidget.h"
#include "ui_gpswidget.h"

#include "gpscontroler.h"
////#include "LCMDATA.hpp"
#include "commondebug.h"

GpsWidget::GpsWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GpsWidget)
{
    ui->setupUi(this);
    //
    m_gpsPortParam.baudRate = 115200;
    m_gpsPortParam.parity = NOPARITY;
    m_gpsPortParam.byteSize = 8;
    m_gpsPortParam.stopBits = ONESTOPBIT;
    ui->comboBoxBaudRate->addItem("115200");
    ui->comboBoxParity->addItem("NOPARITY");
    ui->comboBoxByteSize->addItem("8");
    ui->comboBoxStopBits->addItem("ONESTOPBIT");
    m_nPbOpenPortStatus = PB_OPEN_PORT_STATUS_CLOSED;
    updatePbOpenPortStatus();
    m_nPbShowGpsInfoStatus = PB_SHOW_GPS_INFO_UNSHOW;
    m_nPbSaveGpsStatus = PB_SAVE_GPS_STATUS_CLOSED;
    //ui.text->document ()->setMaximumBlockCount (1000);
    ui->textBrowserShowGpsInfo->document()->setMaximumBlockCount(500);
    m_pRecvGpsInfoThread = new RecvGpsInfoThread();
    connect(m_pRecvGpsInfoThread, SIGNAL(receivedGpsInfoToShowText(const QString&)),
            this, SLOT(receivedGpsInfo(const QString&)));
}

GpsWidget::~GpsWidget()
{
    delete ui;
    delete m_pRecvGpsInfoThread;
}

void GpsWidget::on_pushButtonUpdatePort_clicked()
{
    qDebug("on_pushButtonUpdatePort_clicked");
    updateSerialPort();
}

void GpsWidget::on_pushButtonOpenPort_clicked()
{
    qDebug("on_pushButtonOpenPort_clicked");
    bool bRet = false;
    switch (m_nPbOpenPortStatus)
    {
        case (PB_OPEN_PORT_STATUS_CLOSED):
            // open port
            strncpy(m_gpsPortParam.portName,
                    ui->comboBoxPortName->currentText().toUtf8(),
                    GPS_PORT::MAX_PORT_NAME - 1);
            bRet = m_gpsControler.openGPSPort(m_gpsPortParam);
            if (true == bRet)
            {
                ui->pushButtonOpenPort->setText(tr("Close"));
                m_nPbOpenPortStatus = PB_OPEN_PORT_STATUS_OPENED;
                m_pRecvGpsInfoThread->setGpsControler(&m_gpsControler);
                m_pRecvGpsInfoThread->startThread();
            }
            else
            {
                qDebug("Open serial port failed");
                QMessageBox::warning(this,
                                     tr("Autonomous Driving Control"),
                                     tr("Open serial port failed"),
                                     QMessageBox::Yes);
            }
            break;
        case (PB_OPEN_PORT_STATUS_OPENED):
            // close port
            m_pRecvGpsInfoThread->stopThread();
            m_gpsControler.closeGPSPort();
            ui->pushButtonOpenPort->setText(tr("Open"));
            m_nPbOpenPortStatus = PB_OPEN_PORT_STATUS_CLOSED;
            m_pRecvGpsInfoThread->setGpsControler(NULL);
            break;
        default:
            break;
    }
}

void GpsWidget::on_pushButtonSendCmd_clicked()
{
    if (ui->textEditSendCmd->toPlainText().isEmpty())
        return;
    QString cmd = ui->textEditSendCmd->toPlainText();
    cmd += "\r\n";
    m_gpsControler.sendGPSCmd(cmd.toUtf8().constData());
}

//void GpsWidget::on_pushButtonShowGpsInfo_clicked()
//{
//    switch (m_nPbShowGpsInfoStatus) {
//    case (PB_SHOW_GPS_INFO_UNSHOW):
//        ui->pushButtonShowGpsInfo->setText("Unshow");
//        m_nPbShowGpsInfoStatus = PB_SHOW_GPS_INFO_SHOW;
//        break;
//    case (PB_SHOW_GPS_INFO_SHOW):
//        ui->pushButtonShowGpsInfo->setText("Show");
//        m_nPbShowGpsInfoStatus = PB_SHOW_GPS_INFO_UNSHOW;
//        break;
//    default:
//        break;
//    }
//}

void GpsWidget::on_pushButtonClearGpsInfo_clicked()
{
    ui->textBrowserShowGpsInfo->clear();
}

void GpsWidget::on_pushButtonSaveGps_clicked()
{
    switch (m_nPbSaveGpsStatus)
    {
        case (PB_SAVE_GPS_STATUS_CLOSED):
        {
            QString fileName = QFileDialog::getOpenFileName(this,
                               tr("Save gps log"),
                               ".",
                               tr("gps data files (*.*)"));
            if (!fileName.isEmpty())
            {
                bool bRet = m_pRecvGpsInfoThread->saveGpsDataToFile(fileName.toUtf8().constData());
                if (false == bRet)
                {
                    qDebug("Save gps data to file failed");
                    QMessageBox::warning(this,
                                         tr("Autonomous Driving Control"),
                                         tr("Save gps data to file failed"),
                                         QMessageBox::Yes);
                }
                else
                {
                    ui->pushButtonSaveGps->setText(tr("Stop save"));
                    m_nPbSaveGpsStatus = PB_SAVE_GPS_STATUS_OPENED;
                }
            }
        }
        break;
        case (PB_SAVE_GPS_STATUS_OPENED):
            m_pRecvGpsInfoThread->closeGpsLog();
            ui->pushButtonSaveGps->setText(tr("Save"));
            m_nPbSaveGpsStatus = PB_SAVE_GPS_STATUS_CLOSED;
            break;
        default:
            break;
    }
}

void GpsWidget::receivedGpsInfo(const QString& info)
{
    ui->textBrowserShowGpsInfo->append(info);
    ui->textBrowserShowGpsInfo->moveCursor(QTextCursor::End);
}

void GpsWidget::updateSerialPort()
{
    ui->comboBoxPortName->clear();
    std::vector<SERIAL_INFO> serialInfo;
    EnumPortsWdm(serialInfo);
    for (int i = 0; i < serialInfo.size(); ++i)
        ui->comboBoxPortName->addItem(serialInfo[i].portName);
    updatePbOpenPortStatus();
}

void GpsWidget::updatePbOpenPortStatus()
{
    if (PB_OPEN_PORT_STATUS_CLOSED == m_nPbOpenPortStatus)
    {
        if ((ui->comboBoxPortName->currentText().size() > 0) &&
            (ui->comboBoxBaudRate->currentText().size() > 0) &&
            (ui->comboBoxParity->currentText().size() > 0) &&
            (ui->comboBoxByteSize->currentText().size() > 0) &&
            (ui->comboBoxStopBits->currentText().size() > 0))
            ui->pushButtonOpenPort->setEnabled(true);
        else
            ui->pushButtonOpenPort->setDisabled(true);
    }
}


RecvGpsInfoThread::RecvGpsInfoThread()
{
    m_bRunFlag = false;
    m_bWriteTofile = false;
}

RecvGpsInfoThread::~RecvGpsInfoThread()
{
    stopThread();
}

void RecvGpsInfoThread::startThread()
{
    if (m_bRunFlag)
        return;
    COM_DEBUG(5, ("ADC: RecvGpsInfoThread::startThread, start GPS receiving"));
    m_bRunFlag = true;
    if (NULL != m_pGpsControler)
        m_pGpsControler->enableRecvGps();
    start();
    COM_DEBUG(5, ("ADC: RecvGpsInfoThread::startThread OK"));
}

void RecvGpsInfoThread::stopThread()
{
    if (! m_bRunFlag)
        return;
    m_bRunFlag = false;
    if (NULL != m_pGpsControler)
    {
        COM_DEBUG(5, ("ADC: RecvGpsInfoThread::stopThread, stop GPS receiving"));
        m_pGpsControler->disableRecvGps();
    }
    //terminate();
    bool bRet = wait(5000);
    if (false == bRet)
        COM_ERR("[ERR]ADC: RecvGpsInfoThread::stopThread err");
    else
        COM_DEBUG(5, ("ADC: RecvGpsInfoThread::stopThread OK"));
}

void RecvGpsInfoThread::run()
{
    COM_DEBUG(5, ("ADC: RecvGpsInfoThread::run, receiving gps thread running"));
    GPS_INFO gpsInfo;
    bool bRet = false;
    char serialInfo[512] = { 0 };
    while (m_bRunFlag)
    {
        if (NULL != m_pGpsControler)
        {
            memset(serialInfo, 0, sizeof(serialInfo));
            bRet = m_pGpsControler->recvGPSInfo(gpsInfo, serialInfo, 511);
            if (true == bRet)
            {
                publishGpsDataByLCM(gpsInfo);
                // convert corrdinate
                MAP_COORDINATE coordinate;
                m_pGpsControler->convertCoordinate(gpsInfo.Longitude, gpsInfo.Lattitude, coordinate);
                gpsInfo.Lattitude = coordinate.y;
                gpsInfo.Longitude = coordinate.x;
                if (isValueableInfo(gpsInfo))
                    emit receivedGpsInfoToShowTrace(gpsInfo);
            }
            emit receivedGpsInfoToShowText(serialInfo);
            // write to log file
            if (m_bWriteTofile)
            {
                m_writeFileLoker.lock();
                if (m_gpsLogFile.is_open())
                    m_gpsLogFile << serialInfo << std::endl;
                m_writeFileLoker.unlock();
            }
        }
    }
    COM_DEBUG(5, ("ADC: RecvGpsInfoThread::run, receiving gps thread end"));
}

bool RecvGpsInfoThread::isValueableInfo(const GPS_INFO& info)
{
    static GPS_INFO s_beforeGpsInfo = { 0 };
    if ((abs(info.Lattitude - s_beforeGpsInfo.Lattitude) < 0.000001) &&
        (abs(info.Longitude - s_beforeGpsInfo.Longitude) < 0.000001))
        return (false);
    return (true);
}

void RecvGpsInfoThread::publishGpsDataByLCM(GPS_INFO& gpsInfo)
{
    ////lcmtypes::GPSDATA   gpsdata;
    ////gpsdata.GPS_TIME       = gpsInfo.GPSTime;   // 时间
    ////gpsdata.GPS_HEADING    = gpsInfo.Heading; // 方向角
    ////gpsdata.GPS_LATITUDE   = gpsInfo.Lattitude; // 纬度
    ////gpsdata.GPS_LONGITUDE  = gpsInfo.Longitude; // 经度
    ////gpsdata.GPS_ALTITUDE   = gpsInfo.Altitude;  // 高度
    ////gpsdata.GPS_VE         = gpsInfo.Ve;  // 东向速度
    ////gpsdata.GPS_VN         = gpsInfo.Vn;  // 北向速度
    ////gpsdata.GPS_VU         = gpsInfo.Vu;  // 天向速度
    ////gpsdata.GPS_BASELINE   = gpsInfo.Baseline; // 基线长度
    ////gpsdata.GPS_NSV1       = gpsInfo.NSV1;   // 主天线星数
    ////gpsdata.GPS_NSV2       = gpsInfo.NSV2;   // 辅天线星数
    ////gpsdata.GPS_HDOP       = 0.0f;   // 精度指标
    //////qDebug("Lat = %f,  Lon = %f", gpsdata.GPS_LATITUDE,gpsdata.GPS_LONGITUDE);
    ////m_lcm.publish("GPSDATA", &gpsdata);
}

bool RecvGpsInfoThread::saveGpsDataToFile(const char* fileName)
{
    qDebug("RecvGpsInfoThread::saveGpsDataToFile");
    bool bRet = false;
    if (NULL == fileName)
        return (bRet);
    m_writeFileLoker.lock();
    if (m_gpsLogFile.is_open())
    {
        m_bWriteTofile = false;
        m_gpsLogFile.close();
    }
    m_gpsLogFile.open(fileName);
    if (m_gpsLogFile.is_open())
    {
        m_bWriteTofile = true;
        bRet = true;
    }
    else
        bRet = false;
    m_writeFileLoker.unlock();
    return (bRet);
}

void RecvGpsInfoThread::closeGpsLog(void)
{
    m_writeFileLoker.lock();
    if (m_gpsLogFile.is_open())
    {
        m_bWriteTofile = false;
        m_gpsLogFile.close();
    }
    m_writeFileLoker.unlock();
}
