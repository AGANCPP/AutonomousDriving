#ifndef RECVLCMMSG_H
#define RECVLCMMSG_H

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QThread>
#include <QMutex>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <string>

#include <lcm/lcm-cpp.hpp>
#include <lcm/lcm.h>
#include <lcm/lcm_coretypes.h>

#include "commondebug.h"

/******************************************************************************
* class define
******************************************************************************/
class RecvLcmMsg : public QThread
{
public:
   RecvLcmMsg(const std::string &url);
   RecvLcmMsg();
   ~RecvLcmMsg();

   bool startWork();
   bool stopWork();

   /**
    * Subscribe a callback method of an object to a channel,
    * without automatic message decoding.
    * Be careful, this "handlerMethod" will be call in "RecvLcmMsg message loop thread",
    * please handle the message as quickly as possible, otherwise, will affect ohter message receiving
    */
   template <typename MessageHandlerClass>
   bool subscribe(const std::string& channel,
       void (MessageHandlerClass::*handlerMethod)(const lcm::ReceiveBuffer* rbuf, const std::string& channel),
       MessageHandlerClass* handler);

   template <typename MessageType, typename MessageHandlerClass>
   bool subscribe(const std::string& channel,
       void (MessageHandlerClass::*handlerMethod)(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const MessageType* msg),
       MessageHandlerClass* handler);

protected:
    void run();

private:
    #define RECV_LCM_MSG_INTERNAL_SYS_MSG_CHANNEL "RECV_LCM_MSG_INTERNAL_SYS_MSG_CHANNEL"
    enum {
        INTERNAL_SYS_MSG_ID_QUIT
    };
    typedef struct {
        int msgId;
    }INTERNAL_SYS_MSG;
    void receivedInternalSysMsg(const lcm::ReceiveBuffer* rbuf, const std::string& chan);

private:
    volatile bool m_bRunFlag;
    lcm::LCM      m_lcm;
    char   m_internalSysMsgChannel[128];
};

template <typename MessageHandlerClass>
bool RecvLcmMsg::subscribe(const std::string& channel,
    void (MessageHandlerClass::*handlerMethod)(const lcm::ReceiveBuffer* rbuf, const std::string& channel),
    MessageHandlerClass* handler)
{
    if(!m_lcm.good()) {
        COM_ERR("[ERR]ADC: RecvLcmMsg::subscribe, lcm is not good");
        return (false);
    }

    if (channel == std::string(RECV_LCM_MSG_INTERNAL_SYS_MSG_CHANNEL)) {
        COM_ERR("[ERR]ADC: RecvLcmMsg::subscribe, channel name not good, "
                "because it has been used for internal message, please select another channel name");
        return (false);
    }

    m_lcm.subscribe(channel, handlerMethod, handler);

    return (true);
}

template <typename MessageType, typename MessageHandlerClass>
bool RecvLcmMsg::subscribe(const std::string& channel,
    void (MessageHandlerClass::*handlerMethod)(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const MessageType* msg),
    MessageHandlerClass* handler)
{
    if(!m_lcm.good()) {
        COM_ERR("[ERR]ADC: RecvLcmMsg::subscribe, lcm is not good");
        return (false);
    }

    if (channel == std::string(RECV_LCM_MSG_INTERNAL_SYS_MSG_CHANNEL)) {
        COM_ERR("[ERR]ADC: RecvLcmMsg::subscribe, channel name not good, "
                "because it has been used for internal message, please select another channel name");
        return (false);
    }

    m_lcm.subscribe(channel, handlerMethod, handler);

    return (true);
}

#endif // RECVLCMMSG_H
