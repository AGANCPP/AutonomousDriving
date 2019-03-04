
#include "stdafx.h"

// The next 3 includes are needed for serial port enumeration
#include <objbase.h>
#include <initguid.h>
#include <Setupapi.h>

#include "SerialPortWin32.h"
#include "ComDebug.h"

#pragma comment (lib,"setupapi.lib")

void EnumPortsWdm(std::vector<SERIAL_INFO>& asi)
{
    // Create a device information set that will be the container for
    // the device interfaces.
    GUID* guidDev = (GUID*) &GUID_CLASS_COMPORT;
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVICE_INTERFACE_DETAIL_DATA_A* pDetData = NULL;
    hDevInfo = SetupDiGetClassDevsA(guidDev,
                                    NULL,
                                    NULL,
                                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
                                   );
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        COM_ERR("[ERR] %s,SetupDiGetClassDevs failed. (err=%lx)", __FUNCTION__, GetLastError());
        return;
    }
    SERIAL_INFO serialInfo;
    memset(&serialInfo, 0, sizeof(serialInfo));
    // Enumerate the serial ports
    BOOL bOk = TRUE;
    SP_DEVICE_INTERFACE_DATA ifcData;
    DWORD dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A) + 256;
    pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*) new char[dwDetDataSize];
    // This is required, according to the documentation. Yes,
    // it's weird.
    ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
    for (DWORD ii = 0; bOk; ii++)
    {
        bOk = SetupDiEnumDeviceInterfaces(hDevInfo,
                                          NULL, guidDev, ii, &ifcData);
        if (bOk)
        {
            // Got a device. Get the details.
            SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
            bOk = SetupDiGetDeviceInterfaceDetailA(hDevInfo,
                                                   &ifcData, pDetData, dwDetDataSize, NULL, &devdata);
            if (bOk)
            {
                //CString strDevPath(pDetData->DevicePath);
                strncpy(serialInfo.devPath, pDetData->DevicePath, SERIAL_INFO::MAX_NAME);
                // Got a path to the device. Try to get some more info.
                //TCHAR fname[256];
                //TCHAR desc[256];
                BOOL bSuccess = SetupDiGetDeviceRegistryPropertyA(
                                    hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
                                    (PBYTE)serialInfo.friendlyName, sizeof(serialInfo.friendlyName), NULL);
                bSuccess = bSuccess && SetupDiGetDeviceRegistryPropertyA(
                               hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
                               (PBYTE)serialInfo.portDesc, sizeof(serialInfo.portDesc), NULL);
                //BOOL bUsbDevice = FALSE;
                char locinfo[256];
                if (SetupDiGetDeviceRegistryPropertyA(
                        hDevInfo, &devdata, SPDRP_LOCATION_INFORMATION, NULL,
                        (PBYTE)locinfo, sizeof(locinfo), NULL))
                {
                    // Just check the first three characters to determine
                    // if the port is connected to the USB bus. This isn't
                    // an infallible method; it would be better to use the
                    // BUS GUID. Currently, Windows doesn't let you query
                    // that though (SPDRP_BUSTYPEGUID seems to exist in
                    // documentation only).
                    serialInfo.bUsbDevice = (strcmp(locinfo, "USB") == 0);
                }
                if (bSuccess)
                {
                    // Add an entry to the array
                    //SSerInfo si;
                    //si.strDevPath = strDevPath;
                    //si.strFriendlyName = fname;
                    //si.strPortDesc = desc;
                    //si.bUsbDevice = bUsbDevice;
                    //asi.Add(si);
                    int lenth1 = min(strlen(serialInfo.friendlyName), SERIAL_INFO::MAX_NAME);
                    char* pChar = serialInfo.friendlyName;
                    int index1 = 0;
                    for (index1 = 0; index1 < lenth1; ++index1, ++pChar)
                    {
                        if (*pChar == '(')
                            break;
                    }
                    int index2 = 0;
                    ++pChar;
                    ++index1;
                    int lenth2 = lenth1 - index1;
                    for (index2 = 0; index2 < lenth2; ++index2, ++pChar)
                    {
                        if (*pChar == ')')
                            break;
                    }
                    printf("index1=%d index2=%d lenth1=%d lenth2=%d\n", index1, index2, lenth1, lenth2);
                    if ((index1 < lenth1) && (index2 < lenth2))
                        strncpy(serialInfo.portName, &serialInfo.friendlyName[index1], index2);
                    asi.push_back(serialInfo);
                    memset(&serialInfo, 0, sizeof(serialInfo));
                }
            }
            else
            {
                COM_ERR("[ERR] %s,SetupDiGetDeviceInterfaceDetail failed. (err=%lx)", __FUNCTION__, GetLastError());
                break;
            }
        }
        else
        {
            DWORD err = GetLastError();
            if (err != ERROR_NO_MORE_ITEMS)
            {
                COM_ERR("[ERR] %s,SetupDiEnumDeviceInterfaces failed. (err=%lx)", __FUNCTION__, GetLastError());
                break;
            }
        }
    } // for
    if (pDetData != NULL)
        delete [](char*)pDetData;
    if (hDevInfo != INVALID_HANDLE_VALUE)
        SetupDiDestroyDeviceInfoList(hDevInfo);
}


/*
    CSerialPort
*/
CSerialPort::CSerialPort()
{
    //InitializeCriticalSection(&m_csCommunicationSync);
    strcpy(portParam.portName, "COM1");
    portParam.access = GENERIC_READ | GENERIC_WRITE;    //�������д
    portParam.inQueue = 64;             // ���뻺�����Ĵ�С���ֽ�����
    portParam.outQueue = 64;            // ����������Ĵ�С���ֽ�����
    portParam.baudRate = 9600;          // ������Ϊ9600
    portParam.byteSize = 8;             // ÿ���ֽ���8λ
    portParam.parity = NOPARITY;        // ����żУ��λ
    portParam.stopBits = ONESTOPBIT;    // һ��ֹͣλ
    portParam.evtMask = EV_RXCHAR | EV_TXEMPTY; // ���뻺���������յ�����, ����������е������ѱ���ȫ�ͳ�
    m_hComm = INVALID_HANDLE_VALUE;
}

CSerialPort::~CSerialPort(void)
{
    ClosePort();
    //DeleteCriticalSection(&m_csCommunicationSync);
}

BOOL CSerialPort::InitPort(void)
{
    if (!OpenPort())
        return (FALSE);
    //EnterCriticalSection(&m_csCommunicationSync);
    BOOL bIsSuccess = TRUE;
    /** �ڴ˿���������������Ļ�������С,���������,��ϵͳ������Ĭ��ֵ.
        �Լ����û�������Сʱ,Ҫע�������Դ�һЩ,���⻺�������
    */
    bIsSuccess = SetupComm(m_hComm, portParam.inQueue, portParam.outQueue);
    if (FALSE == bIsSuccess)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] SetupComm err\n");
        return (FALSE);
    }
    /** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */
    COMMTIMEOUTS  CommTimeouts;
    CommTimeouts.ReadIntervalTimeout         = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier  = 0;
    CommTimeouts.ReadTotalTimeoutConstant    = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant   = 0;
    bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
    if (FALSE == bIsSuccess)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] SetCommTimeouts err\n");
        return (FALSE);
    }
    DCB  dcb;
    /** ��ȡ��ǰ�������ò��� */
    bIsSuccess = GetCommState(m_hComm, &dcb);
    if (FALSE == bIsSuccess)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] GetCommState err\n");
        return (FALSE);
    }
    dcb.BaudRate = portParam.baudRate;    // ������
    dcb.ByteSize = portParam.byteSize;    // ÿ���ֽ���?λ
    dcb.Parity = portParam.parity;        // ��żУ��λ
    dcb.StopBits = portParam.stopBits;    // ֹͣλ
    bIsSuccess = SetCommState(m_hComm, &dcb);
    if (FALSE == bIsSuccess)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] SetCommState err\n");
        return (FALSE);
    }
    /**  ��մ��ڻ����� */
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    //LeaveCriticalSection(&m_csCommunicationSync);
    return (TRUE);
}

BOOL CSerialPort::OpenPort(void)
{
    //EnterCriticalSection(&m_csCommunicationSync);
    m_hComm = CreateFileA(portParam.portName,            /** �豸��,COM1,COM2�� */
                          portParam.access,                /** ����ģʽ,��ͬʱ��д */
                          0,                             /** ����ģʽ,0��ʾ������ */
                          NULL,                          /** ��ȫ������,һ��ʹ��NULL */
                          OPEN_EXISTING,                 /** �ò�����ʾ�豸�������,���򴴽�ʧ�� */
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, /* �ص���ʽ */
                          0);                             /* ָ��ģ���ļ��ľ�������˿ڴ��ڴ�״̬ʱ����ʹ�øò�������������ó�0 */
    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] Open serial port err\n");
        return (FALSE);
    }
    //LeaveCriticalSection(&m_csCommunicationSync);
    return (TRUE);
}

void CSerialPort::ClosePort(void)
{
    /** ����д��ڱ��򿪣��ر��� */
    if (m_hComm != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hComm);
        m_hComm = INVALID_HANDLE_VALUE;
    }
}

BOOL CSerialPort::SetSerialComMask(DWORD evtMask_)
{
    BOOL bRet = SetCommMask(m_hComm, evtMask_);
    if (FALSE != bRet)
        portParam.evtMask = evtMask_;
    else
        COM_ERR("[ERR] SetCommMask err\n");
    return (bRet);
}

BOOL CSerialPort::WaitSerialComEvent(DWORD& evtMask_)
{
    OVERLAPPED os;
    BOOL bRet = FALSE;
    DWORD dwTrans, dwError = 0;
    memset(&os, 0, sizeof(OVERLAPPED));
    os.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!(bRet = WaitCommEvent(m_hComm, &evtMask_, &os)))
    {
        if (GetLastError() == ERROR_IO_PENDING)
            bRet = GetOverlappedResult(m_hComm, &os, &dwTrans, TRUE);
    }
    CloseHandle(os.hEvent);
    return (bRet);
}

UINT CSerialPort::GetBytesInCOM()
{
    DWORD dwError = 0;  /** ������ */
    COMSTAT  comstat;   /** COMSTAT�ṹ��,��¼ͨ���豸��״̬��Ϣ */
    memset(&comstat, 0, sizeof(COMSTAT));
    UINT bytesInQue = 0;
    /** �ڵ���ReadFile��WriteFile֮ǰ,ͨ�������������ǰ�����Ĵ����־ */
    if (ClearCommError(m_hComm, &dwError, &comstat))
    {
        bytesInQue = comstat.cbInQue; /** ��ȡ�����뻺�����е��ֽ��� */
    }
    return (bytesInQue);
}

DWORD CSerialPort::ReadData(unsigned char* pData, unsigned int length, DWORD msec_)
{
    COMSTAT ComStat;
    DWORD dwErrorFlags;
    OVERLAPPED osRead;
    DWORD dwBytesRead = 0;
    DWORD dwRet = 0;
    memset(&osRead, 0, sizeof(OVERLAPPED));
    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
    //length = min(length, (DWORD)ComStat.cbInQue);
    //if(!length) {
    //    printf("read buff empty\n");
    //    return (FALSE);
    //}
    BOOL bReadStatus = ReadFile(m_hComm, pData, length, &dwBytesRead, &osRead);
    //printf("bReadStatus=%d\n", bReadStatus);
    if (!bReadStatus)     //���ReadFile��������FALSE
    {
        //GetLastError()��������ERROR_IO_PENDING,�����������ڽ��ж�����
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (msec_ > 0)
            {
                dwRet = WaitForSingleObject(osRead.hEvent, msec_);
                if (WAIT_OBJECT_0 != dwRet)
                    COM_ERR("[ERR] read serial port timeout\n");
                else
                {
                    bReadStatus = GetOverlappedResult(m_hComm, &osRead, &dwBytesRead, TRUE);
                    if (FALSE == bReadStatus)
                        COM_ERR("[ERR] GetOverlappedResult err\n");
                }
            }
        }
    }
    //PurgeComm(m_hComm, PURGE_RXABORT | PURGE_RXCLEAR);
    CloseHandle(osRead.hEvent);
    return (dwBytesRead);
}

DWORD CSerialPort::WriteData(unsigned char* pData, unsigned int length, DWORD msec_)
{
    COMSTAT ComStat;
    DWORD dwErrorFlags;
    DWORD dwBytesWritten = 0;
    OVERLAPPED osWrite;
    BOOL bWriteStat;
    DWORD dwRet = 0;
    memset(&osWrite, 0, sizeof(OVERLAPPED));
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    //printf("hEvent=%p\n", osWrite.hEvent);
    ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
    //PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    bWriteStat = WriteFile(m_hComm, pData, length, &dwBytesWritten, &osWrite);
    //printf("bWriteStat=%d\n", bWriteStat);
    //printf("length=%d dwBytesWritten=%d\n", length, dwBytesWritten);
    if (!bWriteStat)
    {
        //GetLastError()��������ERROR_IO_PENDING,�����������ڽ���д����
        if (GetLastError() == ERROR_IO_PENDING)
        {
            //printf("GetLastError()==ERROR_IO_PENDING\n");
            if (msec_ > 0)
            {
                //printf("WaitForSingleObject\n");
                dwRet = WaitForSingleObject(osWrite.hEvent, msec_);
                if (WAIT_OBJECT_0 != dwRet)
                    COM_ERR("[ERR] write serial port timeout\n");
                else
                {
                    //printf("WaitForSingleObject==WAIT_OBJECT_0\n");
                    bWriteStat = GetOverlappedResult(m_hComm, &osWrite, &dwBytesWritten, TRUE);
                    //printf("length=%d dwBytesWritten=%d\n", length, dwBytesWritten);
                    if (FALSE == bWriteStat)
                        COM_ERR("[ERR] GetOverlappedResult err\n");
                }
            }
        }
    }
    //PurgeComm(m_hComm, PURGE_RXABORT | PURGE_RXCLEAR);
    CloseHandle(osWrite.hEvent);
    return (dwBytesWritten);
}

