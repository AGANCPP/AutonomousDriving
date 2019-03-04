#ifndef RECVGPSDATA_H
#define RECVGPSDATA_H

#include <vector>
#include "CmdLineInterpret.h"
#include "SerialPortWin32.h"

typedef struct {
    double          GPSTime;        // 自本周日 0:00:00 至当前的秒数（格林尼治时间）
    float           Heading;        // 偏航角（0～359.99）
    float           Pitch;          // 俯仰角（-90～90）
    float           Roll;           // 横滚角（-180～180）
    double          Lattitude;      // 纬度（-90～90）
    double          Longitude;      // 经度（-180～180）
    double          Altitude;       // 高度，单位（米）
    float           Ve;             // 东向速度，单位（米/秒）
    float           Vn;             // 北向速度，单位（米/秒）
    float           Vu;             // 天向速度，单位（米/秒）
    float           Baseline;       // 基线长度，单位（米）
    int             NSV1;           // 天线1 卫星数
    int             NSV2;           // 天线2 卫星数
    unsigned char   Status;         // 系统状态 系统状态
}GPS_INFO;

// coordinate
typedef struct {
    double x;
    double y;
}MAP_COORDINATE;

typedef struct {
    enum { MAX_PORT_NAME = 16 };
    char portName[MAX_PORT_NAME];
    unsigned int baudRate;
    unsigned char parity;
    unsigned char byteSize;
    unsigned char stopBits;
}GPS_PORT;

class GPSControler
{
public:
    GPSControler();
    ~GPSControler();

    bool getGpsDataFromFile(const char* fileNameIn, std::vector<GPS_INFO>& gpsDataOut);
    bool analyzeGpsData_XW_GI7660(const char* gpsDataLineIn, GPS_INFO& gpsDataOut);
    bool analyzeGpsData(const char* gpsDataLineIn, GPS_INFO& gpsDataOut)
    {
        return (analyzeGpsData_XW_GI7660(gpsDataLineIn, gpsDataOut));
    }

    bool openGPSPort(GPS_PORT& port);
    void closeGPSPort(void);
    bool isGpsPortOpened(void) { return (m_bIsGpsPortOpened); }
    bool sendGPSCmd(const char *cmd);
    bool recvGPSInfo(GPS_INFO& gpsDataOut, char *dataLineOut = 0, int dataLineLenIn = 0);

    void convertCoordinate(double longitudeIn, double lattitudeIn, MAP_COORDINATE& coordinateOut);

    void enableRecvGps();
    void disableRecvGps();

private:
    CmdLineInterpret m_gpsDataLineInterpret;

    enum { SEND_CMD_TIMEOUT = 3000 };
    CSerialPort m_gpsPort;
    GPS_PORT m_gpsPortParam;
    bool m_bIsGpsPortOpened;
    volatile bool m_bRecvGpsFlag;
//    enum { MAX_DATA_BUFF_SIZE = 1024*4 };
//    char m_gpsDataLineBuff[MAX_DATA_BUFF_SIZE];
};

// other functions
char convertHexCharToDecimal(char hexChar);

#endif // RECVGPSDATA_H
