// VciCanDrive.cpp

#include "VciCanDrive.h"
#include "commondebug.h"

#pragma comment (lib,"../Support/CanLib/ControlCAN.LIB")

VciCanDrive::VciCanDrive()
{
    m_deviceType = 4;   // �豸���ͺ�(USBCAN2)
    m_deviceInd = 0;    // �豸������(ֻ��һ��USBCAN)
    m_canInd = 0;       // �ڼ�·CAN
    m_canConfig.AccCode = 0x00000000;   // ������
    m_canConfig.AccMask = 0xFFFFFFFF;   // ������
    m_canConfig.Reserved = 0;
    m_canConfig.Filter = 1;             // �˲���ʽ��1��ʾ���˲���0��ʾ˫�˲�
    // Timing0��Timing1��������CAN������, 00 1C - 500Kbps
    m_canConfig.Timing0 = 0x00;         // ��ʱ��0
    m_canConfig.Timing1 = 0x1C;         // ��ʱ��1
    m_canConfig.Mode = 0;               // ģʽ��0��ʾ����ģʽ��1��ʾֻ��ģʽ
    m_isConnected = FALSE;
}

VciCanDrive::~VciCanDrive()
{
    // close can di
    this->close();
}

BOOL VciCanDrive::connect(void)
{
    if (FALSE != m_isConnected)
    {
        COM_WARN("[WARN] %s, already connected\n", __FUNCTION__);
        return (TRUE);
    }
    COM_DEBUG(3, ("%s, VCI_OpenDevice(%d, %d, 0)\n", __FUNCTION__, m_deviceType, m_deviceInd));
    if (STATUS_OK != VCI_OpenDevice(m_deviceType, m_deviceInd, 0))
    {
        COM_ERR("[ERR] %s, VCI_OpenDevice err\n", __FUNCTION__);
        return (FALSE);
    }
    COM_DEBUG(3, ("%s, VCI_InitCAN(%d, %d, %d, config)\n", __FUNCTION__, m_deviceType, m_deviceInd, m_canInd));
    if (STATUS_OK != VCI_InitCAN(m_deviceType, m_deviceInd, m_canInd, &m_canConfig))
    {
        COM_ERR("[ERR] %s, VCI_InitCAN err\n", __FUNCTION__);
        VCI_CloseDevice(m_deviceType, m_deviceInd);
        return (FALSE);
    }
    m_isConnected = TRUE;
    return (TRUE);
}

BOOL VciCanDrive::start(void)
{
    if (m_isConnected)
    {
        COM_DEBUG(3, ("%s, VCI_StartCAN(%d, %d, %d)", __FUNCTION__, m_deviceType, m_deviceInd, m_canInd));
        if (STATUS_OK != VCI_StartCAN(m_deviceType, m_deviceInd, m_canInd))
        {
            COM_ERR("[ERR] %s, VCI_StartCAN err\n", __FUNCTION__);
            return (FALSE);
        }
    }
    return (TRUE);
}

BOOL VciCanDrive::reset(void)
{
    if (m_isConnected)
    {
        COM_DEBUG(3, ("%s, VCI_ResetCAN(%d, %d, %d)", __FUNCTION__, m_deviceType, m_deviceInd, m_canInd));
        if (STATUS_OK != VCI_ResetCAN(m_deviceType, m_deviceInd, m_canInd))
        {
            COM_ERR("[ERR] %s, VCI_ResetCAN err\n", __FUNCTION__);
            return (FALSE);
        }
    }
    return (TRUE);
}

BOOL VciCanDrive::close(void)
{
    if (m_isConnected)
    {
        COM_DEBUG(3, ("%s, VCI_CloseDevice(%d, %d)", __FUNCTION__, m_deviceType, m_deviceInd));
        //Sleep(500);
        if (STATUS_OK != VCI_CloseDevice(m_deviceType, m_deviceInd))
        {
            COM_ERR("[ERR] %s, VCI_CloseDevice err\n", __FUNCTION__);
            return (FALSE);
        }
        m_isConnected = FALSE;
    }
    return (TRUE);
}

ULONG VciCanDrive::send(VCI_CAN_OBJ& data, ULONG count)
{
    if (m_isConnected)
    {
        COM_DEBUG(3, ("%s, VCI_Transmit(%d, %d, %d, data, %d)", __FUNCTION__, m_deviceType, m_deviceInd, m_canInd, count));
        return (VCI_Transmit(m_deviceType, m_deviceInd, m_canInd, &data, count));
    }
    else
        COM_ERR("[ERR] %s, have not connect to can\n", __FUNCTION__);
    return (0);
}

ULONG VciCanDrive::receive(VCI_CAN_OBJ& data, ULONG count, INT waitTime)
{
    ULONG nReceiveCount = 0;
    if (m_isConnected)
    {
        COM_DEBUG(3, ("%s, VCI_Transmit(%d, %d, %d, data, %d)", __FUNCTION__, m_deviceType, m_deviceInd, m_canInd, count));
        nReceiveCount = VCI_Receive(m_deviceType, m_deviceInd, m_canInd, &data, count, waitTime);
        if (nReceiveCount <= 0)
        {
            // ע�⣺���û�ж��������������ô˺�������ȡ����ǰ�Ĵ����룬
            // ǧ����ʡ����һ������ʹ����ܲ���֪����������ʲô��
            VCI_ERR_INFO errinfo;
            VCI_ReadErrInfo(m_deviceType, m_deviceInd, m_canInd, &errinfo);
            COM_WARN("[WARN] %s, VCI_Receive err, ErrCode(%08x) Passive_ErrData(%02x %02x %02x) ArLost_ErrData(%02x)\n", __FUNCTION__,
                     errinfo.ErrCode, errinfo.Passive_ErrData[0], errinfo.Passive_ErrData[1], errinfo.Passive_ErrData[2],
                     errinfo.ArLost_ErrData);
        }
    }
    else
        COM_ERR("[ERR] %s, have not connect to can\n", __FUNCTION__);
    return (nReceiveCount);
}
