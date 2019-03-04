// recvoutsidemsg.cpp

/******************************************************************************
 * Head files
 ******************************************************************************/

#include "recvoutsidemsg.h"
#include "commondebug.h"

/******************************************************************************
 * class
 ******************************************************************************/
RecvOutSideMsg::RecvOutSideMsg() :
    m_recvMsgFromLcm("udpm://239.255.76.77:8888?ttl=3"),
    m_recvMsgFromLidar("udpm://239.255.76.67:7667?ttl=3")
{
    m_bWriteTofile = false;


    bool bRet = false;
    bRet = m_recvMsgFromLcm.subscribe("ADC_OUTSIDE_MSG", &RecvOutSideMsg::receivedOutsideMsg, this);
    if(false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::RecvOutSideMsg, subscribe(ADC_OUTSIDE_MSG) error\n");
    }

    bRet = m_recvMsgFromRadar.subscribe("MILLWAVE", &RecvOutSideMsg::receivedFrontRadarMsg, this);
    if(false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::RecvOutSideMsg, subscribe(MILLWAVE) error\n");
    }

    bRet = m_recvMsgFromRadar.subscribe("RADAR", &RecvOutSideMsg::receivedBackRadarMsg, this);
    if(false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::RecvOutSideMsg, subscribe(RADAR) error\n");
    }

    bRet = m_recvMsgFromLidar.subscribe("IBEO", &RecvOutSideMsg::receivedLidarMsg, this);
    if(false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::RecvOutSideMsg, subscribe(IBEO) error\n");
    }
}

RecvOutSideMsg::~RecvOutSideMsg()
{
    //stopWork();
}

void RecvOutSideMsg::startWork()
{
    bool bRet = false;

    bRet = m_recvMsgFromLcm.startWork();
    if (false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::startWork, start receive lcm message error\n");
    }

    bRet = m_recvMsgFromRadar.startWork();
    if (false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::startWork, start receive radar message error\n");
    }

    bRet = m_recvMsgFromLidar.startWork();
    if (false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::startWork, start receive lidar message error\n");
    }
}

void RecvOutSideMsg::stopWork()
{
    bool bRet = m_recvMsgFromLcm.stopWork();
    if (false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::stopWork, stop receive lcm message error\n");
    }

    bRet = m_recvMsgFromRadar.stopWork();
    if (false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::startWork, stop receive radar message error\n");
    }

    bRet = m_recvMsgFromLidar.stopWork();
    if (false == bRet) {
        COM_ERR("[ERR]RecvOutSideMsg::startWork, start receive lidar message error\n");
    }
}

void RecvOutSideMsg::receivedOutsideMsg(const lcm::ReceiveBuffer* rbuf, const std::string& chan)
{
    if ((NULL == rbuf) || (NULL == rbuf->data) || (sizeof(ADC_OUTSIDE_MSG) != rbuf->data_size)) {
        COM_ERR("[ERR]ADC: Receive outside message err\n");
        return;
    }
    ADC_OUTSIDE_MSG msg = { 0 };
    memcpy(&msg, rbuf->data, sizeof(ADC_OUTSIDE_MSG));

    COM_DEBUG(6, ("ADC: Receive outside message [%d]\n", msg.msgId));

    switch (msg.msgId) {
    case (ADC_OUTSIDE_MSG_ID_GPS_INFO):
        handleGpsInfo(msg);
        break;
    default:
        break;
    }

    return;
}

void RecvOutSideMsg::handleGpsInfo(const ADC_OUTSIDE_MSG &msg)
{
    GPS_INFO gpsInfo;

    bool bRet = m_gpsControler.analyzeGpsData(msg.msg.gps.gpsString, gpsInfo);
    if (true == bRet) {
        // convert corrdinate
        MAP_COORDINATE coordinate;
        m_gpsControler.convertCoordinate(gpsInfo.Longitude, gpsInfo.Lattitude, coordinate);
        gpsInfo.Lattitude = coordinate.y;
        gpsInfo.Longitude = coordinate.x;

        if (isValueableInfo(gpsInfo)) {
            emit receivedGpsInfoToShowTrace(gpsInfo);
        }
    }

//    emit receivedGpsInfoToShowText(pMsg->msg.gps.gpsString);

//    // write to log file
//    if (m_bWriteTofile) {
//        m_writeFileLoker.lock();
//        if (m_gpsLogFile.is_open()) {
//            m_gpsLogFile << pMsg->msg.gps.gpsString << std::endl;
//        }
//        m_writeFileLoker.unlock();
//    }
}

bool RecvOutSideMsg::isValueableInfo(const GPS_INFO& info)
{
    static GPS_INFO s_beforeGpsInfo = { 0 };
    if ( (abs(info.Lattitude - s_beforeGpsInfo.Lattitude) < 0.000001) &&
         (abs(info.Longitude - s_beforeGpsInfo.Longitude) < 0.000001) ) {
        return (false);
    }
    return (true);
}

bool RecvOutSideMsg::saveGpsDataToFile(const char* fileName)
{
    COM_DEBUG(5, ("RecvGpsInfoThread::saveGpsDataToFile"));

    bool bRet = false;

    if (NULL == fileName) {
        return (bRet);
    }

    m_writeFileLoker.lock();

    if (m_gpsLogFile.is_open()) {
        m_bWriteTofile = false;
        m_gpsLogFile.close();
    }

    m_gpsLogFile.open(fileName);
    if (m_gpsLogFile.is_open()) {
        m_bWriteTofile = true;
        bRet = true;
    } else {
        bRet = false;
    }

    m_writeFileLoker.unlock();

    return (bRet);
}

void RecvOutSideMsg::closeGpsLog(void)
{
    m_writeFileLoker.lock();

    if (m_gpsLogFile.is_open()) {
        m_bWriteTofile = false;
        m_gpsLogFile.close();
    }

    m_writeFileLoker.unlock();
}

void RecvOutSideMsg::receivedFrontRadarMsg(const lcm::ReceiveBuffer* rbuf,
                                           const std::string& chan,
                                           const lcmtypes::CAN_Data* msg)
{
    if (NULL == msg) {
        COM_ERR("[ERR]ADC: Receive front radar message err\n");
        return;
    }

    FRONT_RADAR_INFO radarInfo = { 0 };
    for (int i = 0 ; i < FRONT_RADAR_INFO::MAX_OBJ_COUNT; ++i) {
        radarInfo.distance[i] = msg->Distance[i];
        radarInfo.xLocation[i] = msg->X_location[i];
        radarInfo.relSpeed[i] = msg->Rel_Speed[i];
    }

    emit frontRadarInfoChanged(radarInfo);
}

void RecvOutSideMsg::receivedBackRadarMsg(const lcm::ReceiveBuffer* rbuf,
                                          const std::string& chan)
{
    if ((NULL == rbuf) || (NULL == rbuf->data) || (sizeof(BACK_RADAR_INFO) != rbuf->data_size)) {
        COM_ERR("[ERR]ADC: Receive back radar message err\n");
        return;
    }

    BACK_RADAR_INFO radarInfo = { 0 };
    memcpy(&radarInfo, rbuf->data, sizeof(BACK_RADAR_INFO));

    //COM_DEBUG(5, ("ADC: receivedBackRadarMsg"));

    emit backRadarInfoChanged(radarInfo);
}

void RecvOutSideMsg::receivedLidarMsg(const lcm::ReceiveBuffer* rbuf,
                                      const std::string& chan,
                                      const lcmtypes::ibeo_t* msg)
{
    if (NULL == msg) {
        COM_ERR("[ERR]ADC: Receive front lidar message err\n");
        return;
    }

    for (int i = 0; i < IBEO_INFO::MAX_OBJECTS; ++i) {
        m_ibeoInfo.ibeoObjects[i].flag = msg->object[i].flag;
        m_ibeoInfo.ibeoObjects[i].objectID = msg->object[i].objectID;
        m_ibeoInfo.ibeoObjects[i].classification = msg->object[i].classification;
        m_ibeoInfo.ibeoObjects[i].classificationAge = msg->object[i].classificationAge;
        m_ibeoInfo.ibeoObjects[i].centerpt_x = msg->object[i].centerpt_x;
        m_ibeoInfo.ibeoObjects[i].centerpt_y = msg->object[i].centerpt_y;
        m_ibeoInfo.ibeoObjects[i].boundingbox_x = msg->object[i].boundingbox_x;
        m_ibeoInfo.ibeoObjects[i].boundingbox_y = msg->object[i].boundingbox_y;
        m_ibeoInfo.ibeoObjects[i].boundingCenter_x = msg->object[i].boundingCenter_x;
        m_ibeoInfo.ibeoObjects[i].boundingCenter_y = msg->object[i].boundingCenter_y;
    }

    emit lidarInfoChanged(m_ibeoInfo);
}

