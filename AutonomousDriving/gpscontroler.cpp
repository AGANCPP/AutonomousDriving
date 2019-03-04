#include <QtGui>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "gpscontroler.h""
#include "commondebug.h"

#define MY_DEBUG qDebug

using namespace std;

GPSControler::GPSControler()
{
    m_gpsDataLineInterpret.SetSeparator(",");

    m_bIsGpsPortOpened = false;
    strncpy(m_gpsPortParam.portName, "COM1", GPS_PORT::MAX_PORT_NAME-1);
    m_gpsPortParam.baudRate = 115200;
    m_gpsPortParam.parity = NOPARITY;
    m_gpsPortParam.byteSize = 8;
    m_gpsPortParam.stopBits = ONESTOPBIT;

    m_bRecvGpsFlag = false;
}

GPSControler::~GPSControler()
{
    //
}

bool GPSControler::getGpsDataFromFile(const char* fileNameIn, std::vector<GPS_INFO>& gpsDataOut)
{
    ifstream infile(fileNameIn);
    if (!infile) {
        MY_DEBUG("Open file error!\n");
        return (false);
    }

    bool bRet = false;
    enum { MAX_DATA_BUFF_SIZE = 512 };
    char gpsDataLineBuff[MAX_DATA_BUFF_SIZE];
    GPS_INFO gpsDataLineInfo;

    // 用于调试
//    ofstream outfile_debug("../../gps_data_debug.txt");

    // 读取文件
    while(true) {
        memset(gpsDataLineBuff, 0, MAX_DATA_BUFF_SIZE);

        if (!infile.getline(gpsDataLineBuff, MAX_DATA_BUFF_SIZE)) {
            break;
        }

        bRet = analyzeGpsData(gpsDataLineBuff, gpsDataLineInfo);
        if (true == bRet) {
            // convert corrdinate
            MAP_COORDINATE coordinate;
            convertCoordinate(gpsDataLineInfo.Longitude, gpsDataLineInfo.Lattitude, coordinate);
            gpsDataLineInfo.Lattitude = coordinate.y;
            gpsDataLineInfo.Longitude = coordinate.x;

            gpsDataOut.push_back(gpsDataLineInfo);

            // 用于调试
//            outfile_debug << setiosflags(ios::fixed) << setprecision(6)
//                          << gpsDataLineInfo.GPSTime << "\t"
//                          << gpsDataLineInfo.Heading << "\t"
//                          << gpsDataLineInfo.Pitch << "\t"
//                          << gpsDataLineInfo.Roll << "\t"
//                          << gpsDataLineInfo.Lattitude << "\t"
//                          << gpsDataLineInfo.Longitude << "\t"
//                          << gpsDataLineInfo.Altitude << "\t"
//                          << gpsDataLineInfo.Ve << "\t"
//                          << gpsDataLineInfo.Vn << "\t"
//                          << gpsDataLineInfo.Vu << "\t"
//                          << gpsDataLineInfo.Status << "\t"
//                          << endl;
        }
    }

    return (true);
}

bool GPSControler::analyzeGpsData_XW_GI7660(const char* gpsDataLineIn, GPS_INFO &gpsDataOut)
{
    vector<string> argTable;

    m_gpsDataLineInterpret.GetAllCommand(gpsDataLineIn, argTable);
    if (argTable.size() < 16) {
        //MY_DEBUG("analyzeGpsData_XW_GI7660, arg size not enough (%d)!\n", argTable.size());
        return (false);
    }

    static const string Header = "$GPFPD";
    if (argTable[0] != Header) {
        //MY_DEBUG("analyzeGpsData_XW_GI7660, Header error (%s)!\n", argTable[0].c_str());
        return (false);
    }

    //double          GPSTime;        // 自本周日 0:00:00 至当前的秒数（格林尼治时间）
    gpsDataOut.GPSTime = atof(argTable[2].c_str());
    //float           Heading;        // 偏航角（0～359.99）
    gpsDataOut.Heading = atof(argTable[3].c_str());
    //float           Pitch;          // 俯仰角（-90～90）
    gpsDataOut.Pitch = atof(argTable[4].c_str());
    //float           Roll;           // 横滚角（-180～180）
    gpsDataOut.Roll = atof(argTable[5].c_str());
    //double          Lattitude;      // 纬度（-90～90）
    gpsDataOut.Lattitude = atof(argTable[6].c_str());
    if (0 == gpsDataOut.Lattitude) {
        return (false);
    }
    //double          Longitude;      // 经度（-180～180）
    gpsDataOut.Longitude = atof(argTable[7].c_str());
    if (0 == gpsDataOut.Longitude) {
        return (false);
    }
    //double          Altitude;       // 高度，单位（米）
    gpsDataOut.Altitude = atof(argTable[8].c_str());
    //float           Ve;             // 东向速度，单位（米/秒）
    gpsDataOut.Ve = atof(argTable[9].c_str());
    //float           Vn;             // 北向速度，单位（米/秒）
    gpsDataOut.Vn = atof(argTable[10].c_str());
    //float           Vu;             // 天向速度，单位（米/秒）
    gpsDataOut.Vu = atof(argTable[11].c_str());
    //float           Baseline;       // 基线长度，单位（米）
    gpsDataOut.Baseline = atof(argTable[12].c_str());
    //int             NSV1;           // 天线1 卫星数
    gpsDataOut.NSV1 = atoi(argTable[13].c_str());
    //int             NSV2;           // 天线2 卫星数
    gpsDataOut.NSV2 = atoi(argTable[14].c_str());
    //unsigned char   Status;         // 系统状态 系统状态
    gpsDataOut.Status = argTable[15].c_str()[1];

    // convert corrdinate
//    MAP_COORDINATE coordinate;
//    convertCoordinate(gpsDataOut.Longitude, gpsDataOut.Lattitude, coordinate);
//    gpsDataOut.Lattitude = coordinate.y;
//    gpsDataOut.Longitude = coordinate.x;

    return (true);
}

void GPSControler::convertCoordinate(double longitudeIn, double lattitudeIn, MAP_COORDINATE& coordinateOut)
{
    coordinateOut.x = longitudeIn*3600*1024;
    coordinateOut.y = lattitudeIn*3600*1024;
}

bool GPSControler::openGPSPort(GPS_PORT& port)
{
    if (m_bIsGpsPortOpened) {
        m_gpsPort.ClosePort();
    }

    strncpy(m_gpsPortParam.portName, port.portName, GPS_PORT::MAX_PORT_NAME-1);
    m_gpsPortParam.baudRate = port.baudRate;
    m_gpsPortParam.parity = port.parity;
    m_gpsPortParam.byteSize = port.byteSize;
    m_gpsPortParam.stopBits = port.stopBits;

    strncpy(m_gpsPort.portParam.portName, m_gpsPortParam.portName, CSerialPort::PORT_PARAM::MAX_NAME-1);
    m_gpsPort.portParam.baudRate = m_gpsPortParam.baudRate;
    m_gpsPort.portParam.parity = m_gpsPortParam.parity;
    m_gpsPort.portParam.byteSize = m_gpsPortParam.byteSize;
    m_gpsPort.portParam.stopBits = m_gpsPortParam.stopBits;
    m_gpsPort.portParam.inQueue = 1024; // 输入缓冲区的大小（字节数）

    BOOL bRet = m_gpsPort.InitPort();
    if (FALSE == bRet) {
        MY_DEBUG("[ERR] %s, InitPort err\n", __FUNCTION__);
        return (false);
    }

    m_bIsGpsPortOpened = true;

    return (true);
}

void GPSControler::closeGPSPort(void)
{
    if (m_bIsGpsPortOpened) {
        m_gpsPort.ClosePort();
    }
}

bool GPSControler::sendGPSCmd(const char* cmd)
{
    if ((NULL == cmd) || (!m_bIsGpsPortOpened)) {
        return (false);
    }

    int cmdLen = strlen(cmd);
    DWORD writeBytes = m_gpsPort.WriteData(cmd, cmdLen, SEND_CMD_TIMEOUT);
    if (writeBytes < cmdLen) {
        MY_DEBUG("[ERR] %s, send command err\n", __FUNCTION__);
        return (false);
    }

    return (true);
}

void GPSControler::enableRecvGps()
{
    m_bRecvGpsFlag = true;
    COM_DEBUG(5, ("ADC: GPSControler::enableRecvGps, run flag %d", m_bRecvGpsFlag));
    m_gpsPort.Start();
}

void GPSControler::disableRecvGps()
{
    m_bRecvGpsFlag = false;
    COM_DEBUG(5, ("ADC: GPSControler::disableRecvGps, run flag %d", m_bRecvGpsFlag));
    m_gpsPort.Stop();
}

 bool GPSControler::recvGPSInfo(GPS_INFO& gpsDataOut, char* dataLineOut, int dataLineLenIn)
 {
     if (!m_bIsGpsPortOpened) {
         return (false);
     }

     bool bRet = false;
     enum { MAX_DATA_BUFF_SIZE = 512 };
     char gpsDataLineBuff[MAX_DATA_BUFF_SIZE];
     //GPS_INFO gpsDataLineInfo;

     // 读取串口
     memset(gpsDataLineBuff, 0, MAX_DATA_BUFF_SIZE);
     // 读取一行
     // 用于写入行缓存
     int index = 0;
     // 用于状态控制
     enum {
         STATUS_WAIT_HEAD,
         STATUS_WAIT_DATA,
         STATUS_WAIT_CHACK,
         STATUS_WAIT_END,
         STATIS_READ_LINE_OK
     };
     int status = STATUS_WAIT_HEAD;
     // 用于校验
     //unsigned char CS = 0;
     //char checkCode[4] = { 0 };
     int checkCodeIndex = 0;

     //int testCount = 0;
     // 开始读取串口（读一行）
     while ((m_bRecvGpsFlag) && (index < MAX_DATA_BUFF_SIZE-1)) {
         char dataBuff[4] = { 0 };
         //COM_DEBUG(5, ("ADC: GPSControler::recvGPSInfo, ReadData"));
         DWORD dataLen = m_gpsPort.ReadData(dataBuff, 1);
         //COM_DEBUG(5, ("ADC: GPSControler::recvGPSInfo, ReadData, data len %d, run flag %d", dataLen, m_bRecvGpsFlag));
         if (0 == dataLen) {
             if (false == m_bRecvGpsFlag) {
                 COM_DEBUG(5, ("ADC: GPSControler::recvGPSInfo, 1. Stop receiving gps"));
                 break;
             }
             DWORD dwEvtMask;
             //COM_DEBUG(5, ("ADC: GPSControler::recvGPSInfo, WaitSerialComEvent"));
             m_gpsPort.WaitSerialComEvent(dwEvtMask);
             if (false == m_bRecvGpsFlag) {
                 COM_DEBUG(5, ("ADC: GPSControler::recvGPSInfo, 2. Stop receiving gps"));
                 break;
             }
         }

         if (1 == dataLen) {
             switch (status) {
             case (STATUS_WAIT_HEAD):
                 if ('$' == dataBuff[0]) {
                     gpsDataLineBuff[index++] = dataBuff[0];
                     status = STATUS_WAIT_DATA;
                 }
                 break;
             case (STATUS_WAIT_DATA):
                 gpsDataLineBuff[index++] = dataBuff[0];
                 if ('*' != dataBuff[0]) {
                     //CS += dataBuff[0];
                 } else {
                     status = STATUS_WAIT_CHACK;
                 }
                 break;
             case (STATUS_WAIT_CHACK):
                 gpsDataLineBuff[index++] = dataBuff[0];
                 //checkCode[checkCodeIndex++] = dataBuff[0];
                 checkCodeIndex++;
                 if (checkCodeIndex > 1) {
                     status = STATUS_WAIT_END;
                 }
                 break;
             case (STATUS_WAIT_END):
                 //gpsDataLineBuff[index++] = dataBuff[0];
                 if ('\n' == dataBuff[0]) {
                     status = STATIS_READ_LINE_OK;
                 }
                 break;
             default:
                 break;
             }

             if (STATIS_READ_LINE_OK == status) {
                 status = STATUS_WAIT_HEAD;
                 bRet = true;
                 break;
             }
         }
     }

     //qDebug(gpsDataLineBuff);

     if (bRet) {
         // 数据校验
         //char checkSum = (convertHexCharToDecimal(checkCode[0]) << 4) + convertHexCharToDecimal(checkCode[1]);
         //qDebug("CS = %d, checkCode = %s, checkSum=%d", CS, checkCode, checkSum);
         //if (CS != checkSum) {
         //    // 校验失败
         //    MY_DEBUG("[ERR] %s, check sum err\n", __FUNCTION__);
         //    return (false);
         //}

         bRet = analyzeGpsData(gpsDataLineBuff, gpsDataOut);

         // 用于调试
         //            outfile_debug << setiosflags(ios::fixed) << setprecision(6)
         //                          << gpsDataLineInfo.GPSTime << "\t"
         //                          << gpsDataLineInfo.Heading << "\t"
         //                          << gpsDataLineInfo.Pitch << "\t"
         //                          << gpsDataLineInfo.Roll << "\t"
         //                          << gpsDataLineInfo.Lattitude << "\t"
         //                          << gpsDataLineInfo.Longitude << "\t"
         //                          << gpsDataLineInfo.Altitude << "\t"
         //                          << gpsDataLineInfo.Ve << "\t"
         //                          << gpsDataLineInfo.Vn << "\t"
         //                          << gpsDataLineInfo.Vu << "\t"
         //                          << gpsDataLineInfo.Status << "\t"
         //                          << endl;

         if ((0 != dataLineOut) && (dataLineLenIn > 0)) {
             strncpy(dataLineOut, gpsDataLineBuff, dataLineLenIn);
         }
     }

     return (bRet);
 }

 // other functions
 char convertHexCharToDecimal(char hexChar)
 {
     if ( ('0' <= hexChar) && (hexChar <= '9') ) {
         return (hexChar - '0');
     } else if ( ('a' <= hexChar) && (hexChar <= 'f') ) {
         return (hexChar - 'a' + 10);
     } else if ( ('A' <= hexChar) && (hexChar <= 'F') ) {
         return (hexChar - 'A' + 10);
     }

     return (0);
 }
