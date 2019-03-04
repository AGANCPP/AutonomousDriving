#ifndef CANWIDGET_H
#define CANWIDGET_H

#include <QWidget>

////#include "cancontrol.h"

namespace Ui
{
    class CanWidget;
}

////class CanControl;
class CanWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanWidget(QWidget* parent = 0);
    ~CanWidget();
public slots:
    void on_pBOpenCan_clicked();
    void on_pBStartRecv_clicked();
    ////void onReceivedCanMsg(const VCI_CAN_OBJ& msg);
private:
    Ui::CanWidget* ui;

    ////CanControl* m_pCanControl;

    enum
    {
        PB_OPEN_CAN_STATUS_CLOSED,
        PB_OPEN_CAN_STATUS_OPENED
    };
    int m_nPbOpenCanStatus;
    enum
    {
        PB_START_RECV_STATUS_STOPPED,
        PB_START_RECV_STATUS_STARTED
    };
    int m_nPbStartRecvStatus;

    enum { MAX_TEMP_STRING_BUFF_SIZE = 512 };
    char m_tempStringBuff[MAX_TEMP_STRING_BUFF_SIZE];
};

#endif // CANWIDGET_H
