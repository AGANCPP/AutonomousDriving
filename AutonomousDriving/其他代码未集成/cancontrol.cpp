// cancontrol.cpp

#include "cancontrol.h"
#include "commondebug.h"

CanControl::CanControl()
{
}

CanControl::~CanControl()
{
    closeCan();
}

bool CanControl::openCan(void)
{
    COM_DEBUG(3, ("%s \n", __FUNCTION__));

    if (isCanOpened()) {
        COM_WARN("[WARN] %s, already initialize\n", __FUNCTION__);
        return (TRUE);
    }

    BOOL bRet = FALSE;
    bRet = m_can.connect();
    if (FALSE == bRet) {
        COM_ERR("[ERR] %s, connect can err\n", __FUNCTION__);
        return (bRet);
    }

    bRet = m_can.start();
    if (FALSE == bRet) {
        m_can.close();
        COM_ERR("[ERR] %s, start can err\n", __FUNCTION__);
        return (bRet);
    }

    return (bRet);
}

bool CanControl::closeCan(void)
{
    COM_DEBUG(3, ("%s \n", __FUNCTION__));

    m_bRecvThreadRunFlag = false;
    if (isCanOpened()) {
        m_can.close();
    }

    return (true);
}

bool CanControl::startRecvCanMsg(void)
{
    COM_DEBUG(3, ("%s \n", __FUNCTION__));

    bool bRet = false;

    if (isCanOpened()) {
        if (!m_bRecvThreadRunFlag) {
            m_bRecvThreadRunFlag = true;
            this->start();
        }
        bRet = true;
    }

    return (bRet);
}

bool CanControl::stopRecvCanMsg(void)
{
    bool bRet = true;
    if (m_bRecvThreadRunFlag) {
        m_bRecvThreadRunFlag = false;
        bRet = this->wait(5000);
        if (false == bRet) {
            COM_ERR("CanControl::stopRecvCanMsg, wait thread exit fail");
        }
    }
    return (bRet);
}

void CanControl::run()
{
    COM_DEBUG(5, ("ADC: CanControl::run, start running"));

    int waitTime = -1;
    ULONG len = 0;

    if (!m_can.isConnected()) {
        COM_WARN("[WARN] can has not been opened\n");
        return;
    }

    Sleep(1);

    while (m_bRecvThreadRunFlag) {
        len = m_can.receive(m_recvBuff[0], MAX_CAN_OBJ, waitTime);
        if (len <= 0) {
            COM_WARN("[WARN] receive can message error\n");
            Sleep(100);
        } else {
            m_recvcount += len;
            COM_DEBUG(3, ("CAN: Received %u frames, amount to %u frames\n", len, m_recvcount));
            for (int i = 0; i < len; ++i) {
                // debug
                COM_DEBUG(3, ("CAN: [%u]ID = %08x\n", i, m_recvBuff[i].ID));
                if(0 == m_recvBuff[i].TimeFlag) {
                    COM_DEBUG(3, ("CAN: [%u]TimeStamp = null\n", i, m_recvBuff[i].TimeFlag));
                } else {
                    COM_DEBUG(3, ("CAN: [%u]TimeStamp = %08x\n", i, m_recvBuff[i].TimeStamp));
                }
                if(0 == m_recvBuff[i].RemoteFlag) {
                    COM_DEBUG(3, ("CAN: [%u]Format = data frame\n", i));
                } else {
                    COM_DEBUG(3, ("CAN: [%u]Format = remote frame\n", i));
                }
                if(0 == m_recvBuff[i].ExternFlag) {
                    COM_DEBUG(3, ("CAN: [%u]Type = standard frame\n", i));
                } else {
                    COM_DEBUG(3, ("CAN: [%u]Type = extensional frame\n", i));
                }
                if (0 == m_recvBuff[i].RemoteFlag) {
                    COM_DEBUG(3, ("CAN: [%u]Data = ", i));
                    if(m_recvBuff[i].DataLen > 8) {
                        m_recvBuff[i].DataLen = 8;
                        for (int j = 0; j < m_recvBuff[i].DataLen; ++j) {
                            COM_DEBUG(3, ("%02x ", m_recvBuff[i].Data[j]));
                        }
                        COM_DEBUG(3, ("\n"));
                    }
                }
                // handle can frame
            }
        }
    }

    COM_DEBUG(5, ("ADC: CanControl::run, stop running"));
}
