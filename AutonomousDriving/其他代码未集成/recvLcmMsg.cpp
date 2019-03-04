// recvLcmMsg.cpp

/******************************************************************************
    Head files
 ******************************************************************************/

#include <fcntl.h>
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#include <errno.h>

#include "recvLcmMsg.h"

/******************************************************************************
    class
 ******************************************************************************/
RecvLcmMsg::RecvLcmMsg(const std::string& url) : m_lcm(url)
{
    m_bRunFlag = false;
    sprintf(m_internalSysMsgChannel, "%s_INSTANCE_%p",
            RECV_LCM_MSG_INTERNAL_SYS_MSG_CHANNEL, this);
    if (m_lcm.good())
        m_lcm.subscribe(m_internalSysMsgChannel, &RecvLcmMsg::receivedInternalSysMsg, this);
    else
        COM_ERR("[ERR]ADC: RecvLcmMsg::RecvLcmMsg, lcm is not good");
}

RecvLcmMsg::RecvLcmMsg()
{
    m_bRunFlag = false;
    sprintf(m_internalSysMsgChannel, "%s_INSTANCE_%p",
            RECV_LCM_MSG_INTERNAL_SYS_MSG_CHANNEL, this);
    if (m_lcm.good())
        m_lcm.subscribe(m_internalSysMsgChannel, &RecvLcmMsg::receivedInternalSysMsg, this);
    else
        COM_ERR("[ERR]ADC: RecvLcmMsg::RecvLcmMsg, lcm is not good");
}

RecvLcmMsg::~RecvLcmMsg()
{
    if (m_bRunFlag)
        stopWork();
}

bool RecvLcmMsg::startWork()
{
    COM_DEBUG(5, ("ADC: RecvLcmMsg::startWork"));
    if (m_bRunFlag)
    {
        COM_WARN("[WARN]ADC: RecvLcmMsg::startWork, alread started");
        return (true);
    }
    if (!m_lcm.good())
    {
        COM_ERR("[ERR]ADC: RecvLcmMsg::startWork, lcm is not good");
        return (false);
    }
    m_bRunFlag = true;
    start();
    COM_DEBUG(5, ("ADC: RecvLcmMsg::startWork OK"));
    return (true);
}

bool RecvLcmMsg::stopWork()
{
    COM_DEBUG(5, ("ADC: RecvLcmMsg::stopWork"));
    if (false == m_bRunFlag)
    {
        COM_WARN("[WARN]ADC: RecvLcmMsg::startWork, alread stopped");
        return (true);
    }
    bool bRet = false;
    if (!m_lcm.good())
    {
        COM_ERR("[ERR]ADC: RecvLcmMsg::stopWork, lcm is not good");
        return (bRet);
    }
    // send quit message
    INTERNAL_SYS_MSG msg = { 0 };
    msg.msgId = INTERNAL_SYS_MSG_ID_QUIT;
    m_lcm.publish(m_internalSysMsgChannel, &msg, sizeof(msg));
    bRet = wait(5000);
    COM_DEBUG(5, ("ADC: RecvLcmMsg::stopWork OK"));
    return (bRet);
}

void RecvLcmMsg::run()
{
    COM_DEBUG(5, ("ADC: RecvLcmMsg::run, start"));
    int fd = 0;
    int status = 0;
    //struct timeval timeout = { 10, 0 };
    fd_set readfds;
    if (!m_lcm.good())
    {
        COM_ERR("[ERR]ADC: RecvOutSideMsg::run, lcm is not good\n");
        return;
    }
    fd = m_lcm.getFileno();
    while (m_bRunFlag)
    {
        //COM_DEBUG(5, ("ADC: \"RecvLcmMsg::run\" wakening"));
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        status = select(fd + 1, &readfds, 0, 0, NULL);   //check fd is readable
        if (-1 == status)
        {
            if (EINTR == errno)
                Sleep(10); /*10ms*/
            else
                break;
        }
        else if (0 == status)
        {
            // time out, continue
            //COM_DEBUG(DEBUG_LEVEL3, ("CAM:timeout!!!(no message frome IPC in three seconds) \r\n"));
        }
        else
        {
            //COM_DEBUG(5, ("CAM: RecvLcmMsg::run received one message"));
            if (FD_ISSET(fd, &readfds))
                m_lcm.handle();
        }
    }
    COM_DEBUG(5, ("ADC: RecvLcmMsg::run, end"));
}

void RecvLcmMsg::receivedInternalSysMsg(const lcm::ReceiveBuffer* rbuf, const std::string& chan)
{
    if ((NULL == rbuf) || (NULL == rbuf->data) || (sizeof(INTERNAL_SYS_MSG) != rbuf->data_size))
    {
        COM_ERR("[ERR]ADC: RecvLcmMsg::receivedInternalSysMsg err\n");
        return;
    }
    INTERNAL_SYS_MSG msg = { 0 };
    memcpy(&msg, rbuf->data, sizeof(INTERNAL_SYS_MSG));
    COM_DEBUG(6, ("ADC: RecvLcmMsg::receivedInternalSysMsg [%d]\n", msg.msgId));
    switch (msg.msgId)
    {
        case (INTERNAL_SYS_MSG_ID_QUIT):
            // Quit lcm message receiving thread
            m_bRunFlag = false;
            break;
        default:
            break;
    }
}
