#include <QtGui>

#include "mainwindow.h"
////#include "naviwidget.h"
#include "gpswidget.h"
////#include "recvoutsidemsg.h"
#include "canwidget.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    // 设置窗体属性
    setWindowTitle(tr("Autonomous Driving Control"));
    //setWindowFlags(Qt::FramelessWindowHint);  // 不显示窗口边框

    setAttribute(Qt::WA_DeleteOnClose);       // 关闭时调用析构函数
    setMinimumSize(QSize(1200, 800));

#if 0
    // 设置列表框
    m_pWidgetSelector = new QListWidget();
    //m_pWidgetSelector->setViewMode(QListView::IconMode);
    m_pWidgetSelector->setViewMode(QListView::IconMode);
    m_pWidgetSelector->setResizeMode(QListView::Adjust);
    m_pWidgetSelector->setTextElideMode(Qt::ElideRight);
    m_pWidgetSelector->setIconSize(QSize(80, 80));
    m_pWidgetSelector->setMovement(QListView::Static);
    m_pWidgetSelector->setMinimumWidth(65);
    m_pWidgetSelector->setMaximumWidth(65);

    m_pWidgetSelector->setSpacing(8);
    m_pWidgetSelector->setStyleSheet("QListWidget{background:#455578}");

    // 设置透明色
    //m_pWidgetSelector->setAttribute(Qt::WA_TranslucentBackground, true);
    //m_pWidgetSelector->setStyleSheet("background-color:transparent");

    //m_pWidgetSelector->setMinimumHeight(480);
    //m_pWidgetSelector->setMaximumHeight(480);
    //m_pWidgetSelector->setSpacing(10);

    //m_pWidgetSelector->insertItem(0, tr("Navi"));
    //m_pWidgetSelector->insertItem(1, tr("GPS"));
    //m_pWidgetSelector->insertItem(2, tr("Radar"));              // millimeter-wave radar
    //m_pWidgetSelector->insertItem(3, tr("Lidar"));              // laser radar
    //m_pWidgetSelector->insertItem(4, tr("Lateral Control"));    // lateral control

    QListWidgetItem *pNaviItem = new QListWidgetItem(m_pWidgetSelector);
    pNaviItem->setSizeHint(QSize(45, 60));
    pNaviItem->setIcon(QIcon(":/image/icon_navi.png"));
    pNaviItem->setText(tr("Navi"));
    pNaviItem->setTextColor(Qt::white);
    //pNaviItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //pNaviItem->setBackgroundColor(QColor(19, 39, 63, 255));
    pNaviItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *pGpsItem = new QListWidgetItem(m_pWidgetSelector);
    pGpsItem->setSizeHint(QSize(45, 60));
    pGpsItem->setIcon(QIcon(":/image/icon_gps.png"));
    pGpsItem->setText(tr("GPS"));
    pGpsItem->setTextColor(Qt::white);
    //pGpsItem->setTextAlignment(Qt::AlignRight);
    pGpsItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *pCanItem = new QListWidgetItem(m_pWidgetSelector);
    pCanItem->setSizeHint(QSize(45, 60));
    pCanItem->setIcon(QIcon(":/image/icon_can.png"));
    pCanItem->setText(tr("CAN"));
    pCanItem->setTextColor(Qt::white);
   //pCanItem->setTextAlignment(Qt::AlignRight);
    pCanItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *pRadarItem = new QListWidgetItem(m_pWidgetSelector);
    pRadarItem->setSizeHint(QSize(45, 60));
    pRadarItem->setIcon(QIcon(":/image/icon_radar.png"));
    pRadarItem->setText(tr("Radar"));
    pRadarItem->setTextColor(Qt::white);
    //pRadarItem->setTextAlignment(Qt::AlignRight);
    pRadarItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *pLidarItem = new QListWidgetItem(m_pWidgetSelector);
    pLidarItem->setSizeHint(QSize(45, 60));
    pLidarItem->setIcon(QIcon(":/image/icon_lidar.png"));
    pLidarItem->setText(tr("Lidar"));
    pLidarItem->setTextColor(Qt::white);
    //pLidarItem->setTextAlignment(Qt::AlignRight);
    pLidarItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *pLateralCtrlItem = new QListWidgetItem(m_pWidgetSelector);
    pLateralCtrlItem->setSizeHint(QSize(45, 60));
    pLateralCtrlItem->setIcon(QIcon(":/image/icon_lateral_control.png"));
    pLateralCtrlItem->setText(tr("Lateral"));
    pLateralCtrlItem->setTextColor(Qt::white);
    //pLateralCtrlItem->setTextAlignment(Qt::AlignRight);
    pLateralCtrlItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    //m_pWidgetSelector->setFlow(QListView::LeftToRight);
#else
    QFont font1;
    font1.setFamily(QString::fromUtf8("Verdana"));
    font1.setPointSize(9);
    QPalette pal;
    pal.setColor(QPalette::Text, QColor(255,255,255));
    m_pWidgetSelector = new QListWidget();
    m_pWidgetSelector->setFixedWidth(120);
    m_pWidgetSelector->setSpacing(12);
    m_pWidgetSelector->setStyleSheet("QListWidget{background:#455578}");
    m_pWidgetSelector->setIconSize(QSize(60, 60));
    m_pWidgetSelector->setFont(font1);
    m_pWidgetSelector->setPalette(pal);

    // tr("<font color='red'>xxxxxxx</font>")
    // tr("<font color='white'>Navi</font>")
    QListWidgetItem *pNaviItem = new QListWidgetItem(QIcon(":/image/icon_navi.png"), tr("Navi"), m_pWidgetSelector);
    QListWidgetItem *pGpsItem = new QListWidgetItem(QIcon(":/image/icon_gps.png"), tr("GPS"), m_pWidgetSelector);
    QListWidgetItem *pCanItem = new QListWidgetItem(QIcon(":/image/icon_can.png"), tr("CAN"), m_pWidgetSelector);
    QListWidgetItem *pRadarItem = new QListWidgetItem(QIcon(":/image/icon_radar.png"), tr("Radar"), m_pWidgetSelector);
    QListWidgetItem *pLidarItem = new QListWidgetItem(QIcon(":/image/icon_lidar.png"), tr("Lidar"), m_pWidgetSelector);
    QListWidgetItem *pLateralCtrlItem = new QListWidgetItem(QIcon(":/image/icon_lateral_control.png"), tr("Control"), m_pWidgetSelector);

    pNaviItem->setFlags(Qt::ItemIsEnabled);
    pGpsItem->setFlags(Qt::ItemIsEnabled);
    pCanItem->setFlags(Qt::ItemIsEnabled);
    pRadarItem->setFlags(Qt::ItemIsEnabled);
    pLidarItem->setFlags(Qt::ItemIsEnabled);
    pLateralCtrlItem->setFlags(Qt::ItemIsEnabled);

    m_pWidgetSelector->insertItem(1, pNaviItem);
    m_pWidgetSelector->insertItem(2, pGpsItem);
    m_pWidgetSelector->insertItem(3, pCanItem);
    m_pWidgetSelector->insertItem(4, pRadarItem);
    m_pWidgetSelector->insertItem(5, pLidarItem);
    m_pWidgetSelector->insertItem(6, pLateralCtrlItem);
#endif

    // 设置堆栈窗体
    ////m_pNaviWidget = new NaviWidget();
    m_pGpsWidget = new GpsWidget();
    m_pCanWidget = new CanWidget();
    //m_pDummyLabel1 = new QLabel(tr("GPS"));
    m_pDummyLabel2 = new QLabel(tr("Radar"));
    m_pDummyLabel3 = new QLabel(tr("Lidar"));
    m_pDummyLabel4 = new QLabel(tr("Control"));

    m_pWidgets = new QStackedWidget();
    ////m_pWidgets->addWidget(m_pNaviWidget);
    m_pWidgets->addWidget(m_pGpsWidget);
    m_pWidgets->addWidget(m_pCanWidget);
    //m_pWidgets->addWidget(m_pDummyLabel1);
    m_pWidgets->addWidget(m_pDummyLabel2);
    m_pWidgets->addWidget(m_pDummyLabel3);
    m_pWidgets->addWidget(m_pDummyLabel4);

    // 设置主窗体布局
    QGridLayout* widgetsLayout = new QGridLayout();
    widgetsLayout->setMargin(2);
    widgetsLayout->setSpacing(0);
    widgetsLayout->addWidget(m_pWidgets);
    widgetsLayout->setAlignment(Qt::AlignCenter);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    ////mainLayout->addWidget(m_pWidgetSelector);
    //mainLayout->addWidget(m_pWidgets, 0, Qt::AlignCenter);
    mainLayout->addLayout(widgetsLayout);
    mainLayout->setStretchFactor(m_pWidgetSelector, 1);
    mainLayout->setStretchFactor(m_pWidgets, 8);
    setLayout(mainLayout);

    // 各窗体间切换
    connect(m_pWidgetSelector, SIGNAL(currentRowChanged(int)), m_pWidgets, SLOT(setCurrentIndex(int)));

    // 接收串口gps消息
    qRegisterMetaType<GPS_INFO>("GPS_INFO");
    ////connect(m_pGpsWidget->m_pRecvGpsInfoThread, SIGNAL(receivedGpsInfoToShowTrace(const GPS_INFO&)),
    ////        m_pNaviWidget, SLOT(updateCurrentTraceSlot(const GPS_INFO&)));

    // 创建lcm消息接收模块
    ////m_pRecvOutsideMsg = new RecvOutSideMsg();
    // 接收LCM gps消息
    ////connect(m_pRecvOutsideMsg, SIGNAL(receivedGpsInfoToShowTrace(const GPS_INFO&)),
    ////        m_pNaviWidget, SLOT(updateCurrentTraceSlot(const GPS_INFO&)));

    ////// 接收雷达消息
    ////qRegisterMetaType<FRONT_RADAR_INFO>("FRONT_RADAR_INFO");
    ////qRegisterMetaType<BACK_RADAR_INFO>("BACK_RADAR_INFO");
    ////qRegisterMetaType<IBEO_INFO>("IBEO_INFO");
    ////connect(m_pRecvOutsideMsg, SIGNAL(frontRadarInfoChanged(const FRONT_RADAR_INFO&)),
    ////        m_pNaviWidget, SLOT(updateFrontRadarInfoSlot(const FRONT_RADAR_INFO&)));
    ////connect(m_pRecvOutsideMsg, SIGNAL(backRadarInfoChanged(const BACK_RADAR_INFO&)),
    ////        m_pNaviWidget, SLOT(updateBackRadarInfoSlot(const BACK_RADAR_INFO&)));
    ////connect(m_pRecvOutsideMsg, SIGNAL(lidarInfoChanged(const IBEO_INFO&)),
    ////        m_pNaviWidget, SLOT(updateLidarInfoSlot(const IBEO_INFO&)));

    // configurate some functions
    configFunctions();


#if 0
    // 设置背景图片
    m_pWidgetSelector->setStyleSheet("QFrame#myframe{border-image:url(:/image/backgroud01.jpg)}" );
    QPixmap pixmap(":/image/backgroud01.jpg");
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(pixmap));
    setPalette(palette);
    setMask(pixmap.mask());
    setAutoFillBackground(true);
#endif
    // 设置透明色
    //setWindowOpacity(0.9);
}

MainWindow::~MainWindow()
{
    qDebug("~MainWindow");
    ////delete (m_pRecvOutsideMsg);
    //SAFE_DELETE(m_pWidgetSelector);
    //SAFE_DELETE(m_pWidgets);
}

void MainWindow::configFunctions(void)
{
    // 启动lcm消息接收线程
    ////m_pRecvOutsideMsg->startWork();
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/image/bg.png"));
}


