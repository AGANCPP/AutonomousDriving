
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
    portParam.access = GENERIC_READ | GENERIC_WRITE;    //允许读和写
    portParam.inQueue = 64;             // 输入缓冲区的大小（字节数）
    portParam.outQueue = 64;            // 输出缓冲区的大小（字节数）
    portParam.baudRate = 9600;          // 波特率为9600
    portParam.byteSize = 8;             // 每个字节有8位
    portParam.parity = NOPARITY;        // 无奇偶校验位
    portParam.stopBits = ONESTOPBIT;    // 一个停止位
    portParam.evtMask = EV_RXCHAR | EV_TXEMPTY; // 输入缓冲区中已收到数据, 输出缓冲区中的数据已被完全送出
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
    /** 在此可以设置输入输出的缓冲区大小,如果不设置,则系统会设置默认值.
        自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢出
    */
    bIsSuccess = SetupComm(m_hComm, portParam.inQueue, portParam.outQueue);
    if (FALSE == bIsSuccess)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] SetupComm err\n");
        return (FALSE);
    }
    /** 设置串口的超时时间,均设为0,表示不使用超时限制 */
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
    /** 获取当前串口配置参数 */
    bIsSuccess = GetCommState(m_hComm, &dcb);
    if (FALSE == bIsSuccess)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] GetCommState err\n");
        return (FALSE);
    }
    dcb.BaudRate = portParam.baudRate;    // 波特率
    dcb.ByteSize = portParam.byteSize;    // 每个字节有?位
    dcb.Parity = portParam.parity;        // 奇偶校验位
    dcb.StopBits = portParam.stopBits;    // 停止位
    bIsSuccess = SetCommState(m_hComm, &dcb);
    if (FALSE == bIsSuccess)
    {
        //LeaveCriticalSection(&m_csCommunicationSync);
        COM_ERR("[ERR] SetCommState err\n");
        return (FALSE);
    }
    /**  清空串口缓冲区 */
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    //LeaveCriticalSection(&m_csCommunicationSync);
    return (TRUE);
}

BOOL CSerialPort::OpenPort(void)
{
    //EnterCriticalSection(&m_csCommunicationSync);
    m_hComm = CreateFileA(portParam.portName,            /** 设备名,COM1,COM2等 */
                          portParam.access,                /** 访问模式,可同时读写 */
                          0,                             /** 共享模式,0表示不共享 */
                          NULL,                          /** 安全性设置,一般使用NULL */
                          OPEN_EXISTING,                 /** 该参数表示设备必须存在,否则创建失败 */
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, /* 重叠方式 */
                          0);                             /* 指向模板文件的句柄，当端口处于打开状态时，不使用该参数，因而必须置成0 */
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
    /** 如果有串口被打开，关闭它 */
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
    DWORD dwError = 0;  /** 错误码 */
    COMSTAT  comstat;   /** COMSTAT结构体,记录通信设备的状态信息 */
    memset(&comstat, 0, sizeof(COMSTAT));
    UINT bytesInQue = 0;
    /** 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的错误标志 */
    if (ClearCommError(m_hComm, &dwError, &comstat))
    {
        bytesInQue = comstat.cbInQue; /** 获取在输入缓冲区中的字节数 */
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
    if (!bReadStatus)     //如果ReadFile函数返回FALSE
    {
        //GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作
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
        //GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行写操作
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

