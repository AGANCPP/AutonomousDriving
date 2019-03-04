#ifndef RECVGPSDATA_H
#define RECVGPSDATA_H

#include <vector>
#include "CmdLineInterpret.h"
#include "SerialPortWin32.h"

typedef struct {
    double          GPSTime;        // �Ա����� 0:00:00 ����ǰ����������������ʱ�䣩
    float           Heading;        // ƫ���ǣ�0��359.99��
    float           Pitch;          // �����ǣ�-90��90��
    float           Roll;           // ����ǣ�-180��180��
    double          Lattitude;      // γ�ȣ�-90��90��
    double          Longitude;      // ���ȣ�-180��180��
    double          Altitude;       // �߶ȣ���λ���ף�
    float           Ve;             // �����ٶȣ���λ����/�룩
    float           Vn;             // �����ٶȣ���λ����/�룩
    float           Vu;             // �����ٶȣ���λ����/�룩
    float           Baseline;       // ���߳��ȣ���λ���ף�
    int             NSV1;           // ����1 ������
    int             NSV2;           // ����2 ������
    unsigned char   Status;         // ϵͳ״̬ ϵͳ״̬
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
