// VciCanDrive.h
#ifndef VCI_CAN_DRIVE_H
#define VCI_CAN_DRIVE_H

#include <Windows.h>
#include "../Support/CanLib/ControlCAN.h"

class VciCanDrive
{
public:
    VciCanDrive();
    ~VciCanDrive();

    BOOL connect(void);
    BOOL start(void);
    BOOL reset(void);
    BOOL close(void);
    ULONG send(VCI_CAN_OBJ& data, ULONG count);
    ULONG receive(VCI_CAN_OBJ& data, ULONG count, INT waitTime = -1);

    bool isConnected(void)
    {
        return (TRUE == m_isConnected);
    }

public:
    // 设备类型号, ie. USBCAN1 3, USBCAN2 4
    DWORD m_deviceType;
    // 设备索引号，当只有一个USBCAN时，索引号为0，有两个时可以为0或1。
    DWORD m_deviceInd;
    // 第几路CAN
    DWORD m_canInd;
    // 初始化参数结构
    VCI_INIT_CONFIG m_canConfig;
    BOOL m_isConnected;
};

#endif // VCI_CAN_DRIVE_H
