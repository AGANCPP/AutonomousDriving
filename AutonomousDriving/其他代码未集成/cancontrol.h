#ifndef CANCONTROL_H
#define CANCONTROL_H

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QThread>
#include <QMutex>

#include "VciCanDrive.h"

/******************************************************************************
* class define
******************************************************************************/
class CanControl : public QThread
{
    Q_OBJECT

public:
    CanControl();
    ~CanControl();

    bool openCan(void);
    bool closeCan(void);

    bool isCanOpened(void) { return (m_can.isConnected()); }

    bool startRecvCanMsg(void);
    bool stopRecvCanMsg(void);

    unsigned long sendToCan(VCI_CAN_OBJ& data, unsigned long count)
    {
        return (m_can.send(data, count));
    }

    unsigned long receiveFromCan(VCI_CAN_OBJ& data, unsigned long count, int waitTime=-1)
    {
        return (m_can.receive(data, count, waitTime));
    }

signals:
    void receivedCanMsg(const VCI_CAN_OBJ& msg);

protected:
    void run();

private:
    VciCanDrive m_can;
    volatile bool m_bRecvThreadRunFlag;

    unsigned long m_recvcount;
    enum { MAX_CAN_OBJ = 50 };
    VCI_CAN_OBJ m_recvBuff[MAX_CAN_OBJ];
};

#endif // CANCONTROL_H
