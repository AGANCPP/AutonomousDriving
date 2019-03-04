
#include "roadscreen.h"
#include "safeDelete.h"

#define ROAD_LINE_WIDTH (5)

RoadScreen::RoadScreen(QWidget* parent) :
    QWidget(parent),
    m_nRoadCount(1),
    m_nRoadIndex(1),
    m_roadChangeFlag(0)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    m_nCarX = 80;
    m_nCarY = 120;
    m_pLabel = new QLabel(this);
    m_pImage = new QImage("./image/car.png");
    *m_pImage = m_pImage->scaled(m_pImage->width() / 3, m_pImage->height() / 3, Qt::KeepAspectRatio);
    m_pLabel->resize(m_pImage->width(), m_pImage->height());
    m_pLabel->setPixmap(QPixmap::fromImage(*m_pImage));
    m_pLabel->show();
    /*******20150120 bootmanager send event to navi (begin)********/
    //QShortcut  *m_Ctl_SetOnChangeRoad= new QShortcut(QKeySequence(tr("Ctrl+L")), this);
    //connect(m_Ctl_SetOnChangeRoad, SIGNAL(activated()), this, SLOT(onOpenChangeRoad()));
    //QShortcut  *m_Ctl_SetOffChangeRoad= new QShortcut(QKeySequence(tr("Ctrl+M")), this);
    //connect(m_Ctl_SetOffChangeRoad, SIGNAL(activated()), this, SLOT(onCloseChangeRoad()));
    QShortcut*  m_Ctl_SetChangeRoad = new QShortcut(QKeySequence(tr("Ctrl+L")), this);
    connect(m_Ctl_SetChangeRoad, SIGNAL(activated()), this, SLOT(handleChangeRoad()));
    connect(this, SIGNAL(clicked(int)), this, SLOT(sendRoadMessage(int)));     //changed road number by mouse
    /*******20150120 bootmanager send event to navi (end)********/
    /******20150306(begin)*****************/
    ////connect(g_strMyApplication, SIGNAL(MsgCamera_WM_CHANGEROAD(int)), this, SLOT(sendRoadMessageByKey(int)));  //changed road number by keyboard
    /******20150306(end)*****************/
}

RoadScreen::~RoadScreen()
{
    SAFE_DELETE(m_pLabel);
    SAFE_DELETE(m_pImage);
}

//void RoadScreen::onAdjustTimerOut(void)
//{
//    m_nRoadCount++;
//    m_nRoadIndex++;
//    if(m_nRoadCount == 4)
//    {
//        m_nRoadCount = 1;
//        m_nRoadIndex = 1;
//    }
//    qDebug("onAdjustTimerOut");
//    update();
//}

void RoadScreen::onChangeRoadScreen(WPARAM nRoadCount, LPARAM nRoadIndex)
{
    m_nRoadCount = (int)nRoadCount;
    m_nRoadIndex = (int)nRoadIndex;
    update();
}

void RoadScreen::changeCarInRoad(int nIndex)
{
}

void RoadScreen::paintEvent(QPaintEvent*)
{
    //return;
    int m_nRoadW = (this->baseSize().width() * 9 / 10);
    int m_nRoadH = (this->baseSize().height() * 9 / 10);
    int m_nRoadX = (this->width() - m_nRoadW) / 2;
    int m_nRoadY = (this->height() - m_nRoadH) / 2;
    if (0 == m_nRoadCount)
        return;
    int m_nRoadSpacing = ((m_nRoadW - ((m_nRoadCount + 1) * ROAD_LINE_WIDTH)) / m_nRoadCount);
    QPainter*  painter = new QPainter(this);
    painter->setRenderHint(QPainter::Antialiasing, true);
    //    painter->setBrush(QBrush(Qt::gray, Qt::SolidPattern));
    painter->setBrush(QBrush(QColor("#153152"), Qt::SolidPattern));
    //if(roadChangeFlag)
    if (1 == m_roadChangeFlag)
        painter->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    painter->drawRect(QRect(0, 0, width(), height()));
    //    painter->setBrush(QBrush(Qt::green, Qt::Dense4Pattern));
    //    painter->drawRect(QRect(m_nRoadX, m_nRoadY, m_nRoadW, m_nRoadH));
    //draw the red dot line
    painter->setPen(QPen(Qt::white, ROAD_LINE_WIDTH, Qt::DotLine));
    for (int i = 0; i <= m_nRoadCount; i++)
    {
        QPoint m_pFront, m_pBack;
        int x = (m_nRoadX + ROAD_LINE_WIDTH / 2 + (ROAD_LINE_WIDTH + m_nRoadSpacing) * i);
        m_pFront.setX(x);
        m_pFront.setY(m_nRoadY);
        m_pBack.setX(x);
        m_pBack.setY(m_nRoadY + m_nRoadH);
        painter->drawLine(m_pFront, m_pBack);
    }
    int nLabelX = (m_nRoadX + ROAD_LINE_WIDTH + (m_nRoadSpacing - m_pImage->width()) / 2 + (m_nRoadIndex - 1) * (m_nRoadSpacing + ROAD_LINE_WIDTH));
    int nLabelY = (m_nRoadY + (m_nRoadH - m_pImage->height()) / 2);
    m_pLabel->move(nLabelX, nLabelY);
    SAFE_DELETE(painter);
}

#if 0
void RoadScreen::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void RoadScreen::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

#endif

/*******20150120 bootmanager send event to navi (begin)********/
//void RoadScreen::onOpenChangeRoad()
//{
//    qDebug("onOpenChangeRoad Ctrl+O is pressed");
//    roadChangeFlag = TRUE;
//    this->setStyleSheet("color:white;background:blue");

//}

//void RoadScreen::onCloseChangeRoad()
//{
//    qDebug("onCloseChangeRoad  Ctrl+C is pressed");
//    roadChangeFlag = FALSE;
//    this->setStyleSheet("color:white;background:#153152");
//}

void RoadScreen::handleChangeRoad()
{
    switch (m_roadChangeFlag)
    {
        case 0:
            qDebug("20150123 handleChangeRoad change road");
            m_roadChangeFlag = 1;
            this->setStyleSheet("color:white;background:blue");
            break;
        case 1:
            qDebug("20150123 handleChangeRoad not change road");
            m_roadChangeFlag = 0;
            this->setStyleSheet("color:white;background:#153152");
            break;
        default:
            break;
    }
}

void RoadScreen::mousePressEvent(QMouseEvent* event)
{
    int m_nRoadW = (this->baseSize().width() * 9 / 10); //??¨º?¦Ì?3¦Ì¦Ì¨¤¦Ì?¡Á¨¹?¨ª?¨¨
    int m_nRoadH = (this->baseSize().height() * 9 / 10); //??¨º?¦Ì?3¦Ì¦Ì¨¤¦Ì?¡Á¨¹???¨¨
    int m_nRoadX = (this->width() - m_nRoadW) / 2;
    int m_nRoadY = (this->height() - m_nRoadH) / 2;
    int m_nRoadSpacing = ((m_nRoadW - ((m_nRoadCount + 1) * ROAD_LINE_WIDTH)) / m_nRoadCount); //????3¦Ì¦Ì¨¤¦Ì??¨ª¡ä?
    //if( roadChangeFlag )
    if (1 == m_roadChangeFlag)
    {
        if (event->button() == Qt::LeftButton)
        {
            int x = event->x();
            int y = event->y();
            if (y > 0 && y < m_nRoadH)
            {
                for (int i = 0; i < m_nRoadCount; i++)
                {
                    if (x > (m_nRoadX + ROAD_LINE_WIDTH / 2 + (ROAD_LINE_WIDTH + m_nRoadSpacing) * i) &&
                        x < (m_nRoadX + ROAD_LINE_WIDTH / 2 + (ROAD_LINE_WIDTH + m_nRoadSpacing) * (i + 1)))
                        emit clicked(i + 1);
                    else
                        qDebug("20150123 mouse clicked area  beyond correct wide area");
                }
            }
        }
    }
}

void RoadScreen::sendRoadMessage(int roadIndex)
{
    int mBootManagerLaneNum = roadIndex;
    COPYDATASTRUCT cpData;
    cpData.dwData = WM_BOOTMANAGER_CHANGELANE;
    cpData.cbData = sizeof(int);
    cpData.lpData = &mBootManagerLaneNum;
    HWND hwindow = ::FindWindowEx(NULL, NULL, NULL, TEXT("MobileNavigator"));
    if (hwindow)
    {
        qDebug("20150123 sendRoadMessage  mBootManagerLaneNum=%d", mBootManagerLaneNum);
        ::SendMessage(hwindow, WM_COPYDATA, (WPARAM)hwindow, (LPARAM)&cpData);
    }
}
/*******20150120 bootmanager send event to navi (end)********/


/******20150306(begin)*******/
void RoadScreen::sendRoadMessageByKey(int direction)
{
    qDebug("======20150306 sendRoadMessageByKey begin.\n");
    COPYDATASTRUCT cpData;
    cpData.dwData = WM_BOOTMANAGER_CHANGELANE;
    cpData.cbData = sizeof(int);
    HWND hwindow = ::FindWindowEx(NULL, NULL, NULL, TEXT("MobileNavigator"));
    switch (direction)
    {
        case HK_NUMPAD1_ID:
            if (1 == m_nRoadCount)
                qDebug("======20150306 road num is one,can not move to left.\n");
            if (1 == m_nRoadIndex)
                qDebug("======20150306 the roadindex is 1 ,can not move to left.\n");
            if ((m_nRoadCount > 1) && (m_nRoadIndex > 1))
            {
                int roadIndex = m_nRoadIndex - 1;
                cpData.lpData = &roadIndex;
                if (hwindow)
                {
                    qDebug("======20150306 remove to left sendRoadMessageByKey  roadIndex=%d", roadIndex);
                    ::SendMessage(hwindow, WM_COPYDATA, (WPARAM)hwindow, (LPARAM)&cpData);
                }
            }
            break;
        case HK_NUMPAD2_ID:
            if (1 == m_nRoadCount)
                qDebug("======20150306 road num is one,can not move to right.\n");
            if (m_nRoadCount == m_nRoadIndex)
                qDebug("======20150306 the roadindex is the last ,can not move to right.\n");
            if ((m_nRoadCount > 1) && (m_nRoadIndex < m_nRoadCount))
            {
                int roadIndex = m_nRoadIndex + 1;
                cpData.lpData = &roadIndex;
                if (hwindow)
                {
                    qDebug("======20150306 remove to right sendRoadMessageByKey  roadIndex=%d", roadIndex);
                    ::SendMessage(hwindow, WM_COPYDATA, (WPARAM)hwindow, (LPARAM)&cpData);
                }
            }
            break;
        default:
            break;
    }
}

/******20150306(end)*******/





