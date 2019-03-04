#ifndef GPSWIDGET_H
#define GPSWIDGET_H

#include <QWidget>
#include <QThread>
#include <QMutex>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <string>

////#include <lcm/lcm-cpp.hpp>
////#include <lcm/lcm.h>
////#include <lcm/lcm_coretypes.h>

#include "gpscontroler.h"

namespace Ui
{
    class GpsWidget;
}

class RecvGpsInfoThread;

class GpsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GpsWidget(QWidget* parent = 0);
    ~GpsWidget();

    void updateSerialPort();
    void updatePbOpenPortStatus();
public slots:
    void on_pushButtonUpdatePort_clicked();
    void on_pushButtonOpenPort_clicked();
    void on_pushButtonSendCmd_clicked();
    //void on_pushButtonShowGpsInfo_clicked();
    void on_pushButtonClearGpsInfo_clicked();
    void on_pushButtonSaveGps_clicked();
    void receivedGpsInfo(const QString& info);
public:
    RecvGpsInfoThread* m_pRecvGpsInfoThread;
private:
    Ui::GpsWidget* ui;

    GPSControler m_gpsControler;
    GPS_PORT m_gpsPortParam;
    enum
    {
        PB_OPEN_PORT_STATUS_CLOSED,
        PB_OPEN_PORT_STATUS_OPENED
    };
    int m_nPbOpenPortStatus;

    enum
    {
        PB_SHOW_GPS_INFO_UNSHOW,
        PB_SHOW_GPS_INFO_SHOW
    };
    int m_nPbShowGpsInfoStatus;

    enum
    {
        PB_SAVE_GPS_STATUS_CLOSED,
        PB_SAVE_GPS_STATUS_OPENED
    };
    int m_nPbSaveGpsStatus;
};

class QThread;

class RecvGpsInfoThread : public QThread
{
    Q_OBJECT
public:
    RecvGpsInfoThread();
    ~RecvGpsInfoThread();

    void setGpsControler(GPSControler* controler)
    {
        m_pGpsControler = controler;
    }

    void startThread();
    void stopThread();
    void publishGpsDataByLCM(GPS_INFO& gpsInfo);

    bool saveGpsDataToFile(const char* fileName);
    void closeGpsLog(void);

signals:
    void receivedGpsInfoToShowText(const QString& info);
    void receivedGpsInfoToShowTrace(const GPS_INFO& info);
protected:
    void run();
private:
    bool isValueableInfo(const GPS_INFO& info);
private:
    GPSControler* m_pGpsControler;
    volatile bool m_bRunFlag;
    ////lcm::LCM      m_lcm;

    QMutex m_writeFileLoker;
    bool m_bWriteTofile;
    std::fstream m_gpsLogFile;
};

#endif // GPSWIDGET_H
