// VciCanDrive.cpp

#include "VciCanDrive.h"
#include "commondebug.h"

#pragma comment (lib,"../Support/CanLib/ControlCAN.LIB")

VciCanDrive::VciCanDrive()
{
    m_deviceType = 4;   // 设备类型号(USBCAN2)
    m_deviceInd = 0;    // 设备索引号(只有一个USBCAN)
    m_canInd = 0;       // 第几路CAN
    m_canConfig.AccCode = 0x00000000;   // 验收码
    m_canConfig.AccMask = 0xFFFFFFFF;   // 屏蔽码
    m_canConfig.Reserved = 0;
    m_canConfig.Filter = 1;             // 滤波方式，1表示单滤波，0表示双滤波
    // Timing0和Timing1用来设置CAN波特率, 00 1C - 500Kbps
    m_canConfig.Timing0 = 0x00;         // 定时器0
    m_canConfig.Timing1 = 0x1C;         // 定时器1
    m_canConfig.Mode = 0;               // 模式，0表示正常模式，1表示只听模式
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
            // 注意：如果没有读到数据则必须调用此函数来读取出当前的错误码，
            // 千万不能省略这一步（即使你可能不想知道错误码是什么）
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
