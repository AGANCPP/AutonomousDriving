#ifndef RECVOUTSIDEMSG_H
#define RECVOUTSIDEMSG_H

/******************************************************************************
    Head files
 ******************************************************************************/
#include "recvLcmMsg.h"

#include "gpscontroler.h"
#include "CAN_Data.hpp"
#include "radardata.h"
#include "ibeo_t.hpp"

/******************************************************************************
    message define
 ******************************************************************************/
enum
{
    ADC_OUTSIDE_MSG_ID_GPS_INFO
};

typedef struct ADC_OUTSIDE_MSG_
{
    int msgId;

    union
    {
        struct
        {
            enum { GPS_STRING_SIZE = 512 };
            char gpsString[GPS_STRING_SIZE];
        } gps;
    } msg;
} ADC_OUTSIDE_MSG;

/******************************************************************************
    class define
 ******************************************************************************/
class RecvOutSideMsg : public QObject
{
    Q_OBJECT
public:
    RecvOutSideMsg();
    ~RecvOutSideMsg();

    void startWork();
    void stopWork();
    //void publishGpsDataByLCM(GPS_INFO &gpsInfo);

    bool saveGpsDataToFile(const char* fileName);
    void closeGpsLog(void);

signals:
    void receivedGpsInfoToShowText(const QString& info);
    void receivedGpsInfoToShowTrace(const GPS_INFO& info);
    void frontRadarInfoChanged(const FRONT_RADAR_INFO& info);
    void backRadarInfoChanged(const BACK_RADAR_INFO& info);
    void lidarInfoChanged(const IBEO_INFO& info);

private:
    bool isValueableInfo(const GPS_INFO& info);
    void receivedOutsideMsg(const lcm::ReceiveBuffer* rbuf, const std::string& chan);
    void receivedFrontRadarMsg(const lcm::ReceiveBuffer* rbuf, const std::string& chan, const lcmtypes::CAN_Data* msg);
    void receivedBackRadarMsg(const lcm::ReceiveBuffer* rbuf, const std::string& chan);
    void receivedLidarMsg(const lcm::ReceiveBuffer* rbuf, const std::string& chan, const lcmtypes::ibeo_t* msg);

    void handleGpsInfo(const ADC_OUTSIDE_MSG& msg);

private:
    GPSControler m_gpsControler;
    RecvLcmMsg m_recvMsgFromLcm;
    RecvLcmMsg m_recvMsgFromRadar;
    RecvLcmMsg m_recvMsgFromLidar;

    IBEO_INFO m_ibeoInfo;

    QMutex m_writeFileLoker;
    bool m_bWriteTofile;
    std::fstream m_gpsLogFile;
};

#endif // RECVOUTSIDEMSG_H
