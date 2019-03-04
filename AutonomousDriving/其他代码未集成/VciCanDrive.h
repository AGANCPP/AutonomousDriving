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
    // �豸���ͺ�, ie. USBCAN1 3, USBCAN2 4
    DWORD m_deviceType;
    // �豸�����ţ���ֻ��һ��USBCANʱ��������Ϊ0��������ʱ����Ϊ0��1��
    DWORD m_deviceInd;
    // �ڼ�·CAN
    DWORD m_canInd;
    // ��ʼ�������ṹ
    VCI_INIT_CONFIG m_canConfig;
    BOOL m_isConnected;
};

#endif // VCI_CAN_DRIVE_H
