// SerialPortWin32.h
#ifndef SERIAL_PORT_WIN32_H
#define SERIAL_PORT_WIN32_H
#include <Windows.h>
#include <vector>

typedef struct _SERIAL_INFO
{
    enum { MAX_NAME = 256 };
    char devPath[MAX_NAME];          // Device path for use with CreateFile()
    char portName[MAX_NAME];         // Simple name (i.e. COM1)
    char friendlyName[MAX_NAME];     // Full name to be displayed to a user
    char portDesc[MAX_NAME];         // friendly name without the COMx
    BOOL  bUsbDevice;                 // Provided through a USB connection?
} SERIAL_INFO;

void EnumPortsWdm(std::vector<SERIAL_INFO>& asi);

/*
    CSerialPort
*/
class CSerialPort
{
public:
    CSerialPort(void);
    ~CSerialPort(void);

public:
    typedef struct
    {
        enum { MAX_NAME = 64 };
        char portName[MAX_NAME];
        DWORD access;
        DWORD inQueue;
        DWORD outQueue;
        DWORD baudRate;
        BYTE  byteSize;
        BYTE  parity;
        BYTE  stopBits;
        DWORD evtMask;
    } PORT_PARAM;
    PORT_PARAM portParam;

    BOOL  InitPort(void);
    void  ClosePort(void);
    DWORD ReadData(char* pData, int length, DWORD msec_ = INFINITE);
    DWORD WriteData(const char* pData, int length, DWORD msec_ = INFINITE);
    UINT  GetBytesInCOM();

    BOOL  SetSerialComMask(DWORD evtMask_);
    DWORD GetSerialComMask(void)
    {
        return (portParam.evtMask);
    }
    BOOL  WaitSerialComEvent(DWORD& evtMask_);

    void Start(void);
    void Stop(void);

private:
    BOOL OpenPort(void);

private:
    HANDLE  m_hComm;
    HANDLE  m_hEventUsedForComEvent;
    HANDLE  m_hEventUsedForWrite;
    HANDLE  m_hEventUsedForRead;
    CRITICAL_SECTION   m_csCommunicationSync;
    volatile int m_bRunFlag;
};

#endif // SERIAL_PORT_WIN32_H

