
#include "canwidget.h"
#include "ui_canwidget.h"
#include "commondebug.h"

CanWidget::CanWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CanWidget)
{
    ui->setupUi(this);
    ////m_pCanControl = new CanControl();
    ////m_nPbOpenCanStatus = PB_OPEN_CAN_STATUS_CLOSED;
    ////m_nPbStartRecvStatus = PB_START_RECV_STATUS_STOPPED;
    ////ui->textBrowserCanMsg->document()->setMaximumBlockCount(500);
    ////qRegisterMetaType<VCI_CAN_OBJ>("VCI_CAN_OBJ");
    ////connect(m_pCanControl, SIGNAL(receivedCanMsg(const VCI_CAN_OBJ&)), this, SLOT(onReceivedCanMsg(const VCI_CAN_OBJ&)));
}

CanWidget::~CanWidget()
{
    delete ui;
    ////delete m_pCanControl;
}

void CanWidget::on_pBOpenCan_clicked()
{
    ////switch (m_nPbOpenCanStatus)
    ////{
    ////    case (PB_OPEN_CAN_STATUS_CLOSED):
    ////        COM_DEBUG(5, ("ADC: CanWidget::on_pBOpenCan_clicked, Open CAN"));
    ////        if (m_pCanControl->openCan())
    ////        {
    ////            ui->pBOpenCan->setText(tr("Close Can"));
    ////            m_nPbOpenCanStatus = PB_OPEN_CAN_STATUS_OPENED;
    ////            COM_DEBUG(5, ("ADC: CanWidget::on_pBOpenCan_clicked, Open CAN OK"));
    ////        }
    ////        else
    ////        {
    ////            COM_ERR("Open CAN failed");
    ////            QMessageBox::warning(this,
    ////                                 tr("Autonomous Driving Control"),
    ////                                 tr("Open CAN failed"),
    ////                                 QMessageBox::Yes);
    ////        }
    ////        break;
    ////    case (PB_OPEN_CAN_STATUS_OPENED):
    ////        COM_DEBUG(5, ("ADC: CanWidget::on_pBOpenCan_clicked, Close CAN"));
    ////        if (m_pCanControl->closeCan())
    ////        {
    ////            ui->pBOpenCan->setText(tr("Open Can"));
    ////            m_nPbOpenCanStatus = PB_OPEN_CAN_STATUS_CLOSED;
    ////            COM_DEBUG(5, ("ADC: CanWidget::on_pBOpenCan_clicked, Close CAN OK"));
    ////        }
    ////        else
    ////        {
    ////            COM_ERR("Close CAN failed");
    ////            QMessageBox::warning(this,
    ////                                 tr("Autonomous Driving Control"),
    ////                                 tr("Close CAN failed"),
    ////                                 QMessageBox::Yes);
    ////        }
    ////        break;
    ////    default:
    ////        break;
    ////}
}

void CanWidget::on_pBStartRecv_clicked()
{
    ////switch (m_nPbStartRecvStatus)
    ////{
    ////    case (PB_START_RECV_STATUS_STOPPED):
    ////        COM_DEBUG(5, ("ADC: CanWidget::on_pBStartRecv_clicked, Start receive message from CAN"));
    ////        if (m_pCanControl->startRecvCanMsg())
    ////        {
    ////            ui->pBStartRecv->setText(tr("Stop Receive"));
    ////            m_nPbStartRecvStatus = PB_START_RECV_STATUS_STARTED;
    ////            COM_DEBUG(5, ("ADC: CanWidget::on_pBStartRecv_clicked, Start receive message from CAN OK"));
    ////        }
    ////        else
    ////        {
    ////            COM_ERR("Start receive message from CAN failed");
    ////            QMessageBox::warning(this,
    ////                                 tr("Autonomous Driving Control"),
    ////                                 tr("Start receive message from CAN failed"),
    ////                                 QMessageBox::Yes);
    ////        }
    ////        break;
    ////    case (PB_START_RECV_STATUS_STARTED):
    ////        COM_DEBUG(5, ("ADC: CanWidget::on_pBStartRecv_clicked, Stop receive message from CAN"));
    ////        if (m_pCanControl->stopRecvCanMsg())
    ////        {
    ////            ui->pBStartRecv->setText(tr("Start Receive"));
    ////            m_nPbStartRecvStatus = PB_START_RECV_STATUS_STOPPED;
    ////            COM_DEBUG(5, ("ADC: CanWidget::on_pBStartRecv_clicked, Stop receive message from CAN OK"));
    ////        }
    ////        else
    ////        {
    ////            COM_ERR("Stop receive message from CAN failed");
    ////            QMessageBox::warning(this,
    ////                                 tr("Autonomous Driving Control"),
    ////                                 tr("Stop receive message from CAN failed"),
    ////                                 QMessageBox::Yes);
    ////        }
    ////        break;
    ////    default:
    ////        break;
    ////}
}

////void CanWidget::onReceivedCanMsg(const VCI_CAN_OBJ& msg)
////{
////    sprintf(m_tempStringBuff, "%08x %08x %02x %02x %02x %02x %02x "
////            "%02x %02x %02x %02x %02x %02x %02x %02x",
////            msg.ID, msg.TimeStamp, msg.TimeFlag, msg.SendType, msg.RemoteFlag, msg.ExternFlag, msg.DataLen,
////            msg.Data[0], msg.Data[1], msg.Data[2], msg.Data[3], msg.Data[4], msg.Data[5], msg.Data[6], msg.Data[7]);
////    ui->textBrowserCanMsg->append(m_tempStringBuff);
////    ui->textBrowserCanMsg->moveCursor(QTextCursor::End);
////}

