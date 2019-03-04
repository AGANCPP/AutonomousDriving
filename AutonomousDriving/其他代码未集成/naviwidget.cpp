// naviwidget.cpp

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QtGui>

#include "naviwidget.h"
#include "navimapwidget.h"
#include "navidrivingwidget.h"
#include "commondebug.h"

/******************************************************************************
 * class
 ******************************************************************************/
NaviWidget::NaviWidget(QWidget *parent) : QWidget(parent)
{
    // Crate map widget
    m_pMapWidget = new MapWidget(&getNaviData(), this);
    m_pMapWidget->setNaviData(&getNaviData());
    // Create driving map widget
    m_pDrivingMapWidget = new DrivingMapWidget(&getNaviData(), this);
    m_pDrivingMapWidget->setNaviData(&getNaviData());

    // Create actions
    m_pLoadMapAction = new QAction(tr("Load Map"), this);
    m_pLoadMapAction->setStatusTip(tr("Load Map file"));
    connect(m_pLoadMapAction, SIGNAL(triggered()), this, SLOT(loadMapFileSlot()));

    m_pLoadHistoryTraceAction = new QAction(tr("Load History Trace"), this);
    m_pLoadHistoryTraceAction->setStatusTip(tr("Load History Trace file"));
    connect(m_pLoadHistoryTraceAction, SIGNAL(triggered()), this, SLOT(loadHistoryTraceSlot()));

    m_pOpenCamCaptureAction = new QAction(tr("Open Camera Capture"), this);
    m_pOpenCamCaptureAction->setStatusTip(tr("Open Camera Capture module"));
    connect(m_pOpenCamCaptureAction, SIGNAL(triggered()), this, SLOT(onOpenCamCapture()));
    m_nPbOpenCamStatus = PB_OPEN_CAM_STATUS_CLOSED;

    // Crate camera image widget
    m_pCamCaptureControl = new (CamControl);
    connect(m_pCamCaptureControl, SIGNAL(camRecoRetImgChanged(uchar*, int, int, int, float, float)),
            this, SLOT(onCamRecoRetImgChanged(uchar*, int, int, int, float, float)));
//    m_pCamSignImgDisplayWidget = new ImgDisplayWidget(this);
//    m_pCamLaneImgDisplayWidget = new ImgDisplayWidget(this);
//    connect(m_pCamCaptureControl, SIGNAL(camSignImgChanged(uchar*, int, int)),
//            m_pCamSignImgDisplayWidget, SLOT(onShowScreen(uchar*, int, int)));
//    connect(m_pCamCaptureControl, SIGNAL(camLaneImgChanged(uchar*, int, int)),
//            m_pCamLaneImgDisplayWidget, SLOT(onShowScreen(uchar*, int, int)));
    m_pShowCamInfo = new ShowCamInfo(this);
    connect(m_pCamCaptureControl, SIGNAL(camSignImgChanged(uchar*, int, int)),
            m_pShowCamInfo, SLOT(onShowSignImg(uchar*, int, int)));
    connect(m_pCamCaptureControl, SIGNAL(camLaneImgChanged(uchar*, int, int)),
            m_pShowCamInfo, SLOT(onShowLaneImg(uchar*, int, int)));
    connect(m_pCamCaptureControl, SIGNAL(showCamRunningInfo(const QString&, int)),
            m_pShowCamInfo, SLOT(onShowCamRunningInfo(const QString&, int)));

    // Create menus
    m_pNaviMenuBar = new QMenuBar(this);
    m_pNaviMenuBar->resize(51, 51);
    //m_pNaviMenuBar->setStyleSheet("QMenu { backgroud-color: white; }");
    m_pNaviMemu = m_pNaviMenuBar->addMenu(QIcon(":/image/icon_menu.png"),tr("Menu"));
    //m_pNaviMemu->resize(51, 51);
    //m_pNaviMemu->setIcon(QIcon(":/image/icon_menu.png"));

    m_pNaviMemu->addAction(m_pLoadMapAction);
    m_pNaviMemu->addAction(m_pLoadHistoryTraceAction);
    m_pNaviMemu->addAction(m_pOpenCamCaptureAction);
    //m_pNaviMenuBar->adjustSize();

    // Create buttons
    QFont font1;
    font1.setFamily(QString::fromUtf8("Verdana"));
    font1.setPointSize(9);

    m_pPbChangeToMapView = new QPushButton(this);
    m_pPbChangeToMapView->setMinimumHeight(50);
    m_pPbChangeToMapView->setMaximumHeight(50);
    m_pPbChangeToMapView->setMinimumWidth(120);
    m_pPbChangeToMapView->setMaximumWidth(120);
    m_pPbChangeToMapView->setText(tr("Map"));
    m_pPbChangeToMapView->setFont(font1);
    m_pPbChangeToMapView->setIcon(QIcon(":/image/icon_map.png"));
    QPalette palBtnMapView = m_pPbChangeToMapView->palette();
    palBtnMapView.setColor(QPalette::Button,Qt::white);
    m_pPbChangeToMapView->setPalette(palBtnMapView);
    m_pPbChangeToMapView->setAutoFillBackground(true);
    m_pPbChangeToMapView->setFlat(true);
    connect(m_pPbChangeToMapView, SIGNAL(clicked()), this, SLOT(onChangeToMapView()));

    m_pPbChangeToCamView = new QPushButton(this);
    m_pPbChangeToCamView->setMinimumHeight(50);
    m_pPbChangeToCamView->setMaximumHeight(50);
    m_pPbChangeToCamView->setMinimumWidth(120);
    m_pPbChangeToCamView->setMaximumWidth(120);
    m_pPbChangeToCamView->setText(tr("Camera"));
    m_pPbChangeToCamView->setFont(font1);
    m_pPbChangeToCamView->setIcon(QIcon(":/image/icon_camera.png"));
    QPalette palBtnCamView = m_pPbChangeToCamView->palette();
    palBtnCamView.setColor(QPalette::Button,Qt::white);
    m_pPbChangeToCamView->setPalette(palBtnCamView);
    m_pPbChangeToCamView->setAutoFillBackground(true);
    m_pPbChangeToCamView->setFlat(true);
    connect(m_pPbChangeToCamView, SIGNAL(clicked()), this, SLOT(onChangeToCamView()));

    m_pPbStartAutoDriving = new QPushButton(this);
    m_pPbStartAutoDriving->setFont(font1);
    m_pPbStartAutoDriving->setText(tr("Start"));
    m_pPbStartAutoDriving->setMinimumHeight(50);
    m_pPbStartAutoDriving->setMaximumHeight(50);
    m_pPbStartAutoDriving->setMinimumWidth(120);
    m_pPbStartAutoDriving->setMaximumWidth(120);
    m_pPbStartAutoDriving->setIcon(QIcon(":/image/icon_start.png"));
    QPalette palBtnStartAutoDriving = m_pPbStartAutoDriving->palette();
    palBtnStartAutoDriving.setColor(QPalette::Button,Qt::white);
    m_pPbStartAutoDriving->setPalette(palBtnStartAutoDriving);
    m_pPbStartAutoDriving->setAutoFillBackground(true);
    m_pPbStartAutoDriving->setFlat(true);
    connect(m_pPbStartAutoDriving, SIGNAL(clicked()), this, SLOT(onStartAutoDriving()));
    m_nPbAutoDrivingStatus = PB_START_AUTO_DRIVING_STATUS_STOPPED;

    // 状态栏
    m_pLabelShowStatusInfo = new QLabel(this);
    m_pLabelShowStatusInfo->setAutoFillBackground(true);
    m_pLabelShowStatusInfo->setFont(font1);
    m_pLabelShowStatusInfo->setAlignment(Qt::AlignCenter);
    onShowStatusInfo("Normal", 2);
    connect(m_pDrivingMapWidget, SIGNAL(showInfoToStatusBar(const QString&, int)), this, SLOT(onShowStatusInfo(const QString&, int)));

    m_nViewStatus = VIEW_STATUS_MAP;
}

NaviWidget::~NaviWidget()
{
    delete (m_pCamCaptureControl);
}

void NaviWidget::resizeEvent(QResizeEvent *)
{
    // setting layout
    updateView();
}

void NaviWidget::dummySlot(void)
{
    COM_DEBUG(5, ("NaviWidget::dummySlot"));
}

void NaviWidget::loadMapFileSlot(void)
{
    COM_DEBUG(5, ("NaviWidget::loadMapFileSlot"));

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open map file"),
                                                    ".",
                                                    tr("sqlit files (*.sqlit)"));
    if (!fileName.isEmpty()) {
        bool bRet = getNaviData().m_mapData.loadMapData(fileName);
        if (false == bRet) {
            COM_WARN("[WARN] load map file (%s) error", fileName.toUtf8().constData());
            QMessageBox::warning(this,
                                 tr("Autonomous Driving Control"),
                                 tr("Load map file error"),
                                 QMessageBox::Yes);

        } else {
            COM_DEBUG(5, ("load map file (%s) ok", fileName.toUtf8().constData()));

            m_pDrivingMapWidget->reqUpdateMapFrame();
            m_pMapWidget->reqUpdateMapFrame();
        }
    }
}

void NaviWidget::loadHistoryTraceSlot(void)
{
    COM_DEBUG(5, ("NaviWidget::loadHistoryTraceSlot"));

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open History Trace"),
                                                    ".",
                                                    tr("gps data files (*.htrace)"));
    if (!fileName.isEmpty()) {
        bool bRet = getNaviData().m_historyTrace.loadHistoryTrace(fileName);
        if (false == bRet) {
            COM_WARN("[WARN] Load History Trace file (%s) error", fileName.toUtf8().constData());
            QMessageBox::warning(this,
                                 tr("Autonomous Driving Control"),
                                 tr("Load History Trace file error"),
                                 QMessageBox::Yes);

        } else {
            COM_DEBUG(5, ("Load History Trace file (%s) ok", fileName.toUtf8().constData()));

            m_pDrivingMapWidget->reqUpdateHistoryTraceFrame();
            m_pMapWidget->reqUpdateHistoryTraceFrame();
        }
    }
}

void NaviWidget::updateCurrentTraceSlot(const GPS_INFO &position)
{
//    qDebug("NaviWidget::updateCurrentTraceSlot (%f, %f)", position.Longitude, position.Lattitude);
    CurrentTrace& currentTrace = getNaviData().m_currentTrace;
    currentTrace.updateCurrentTrace(position);

    m_pDrivingMapWidget->reqUpdateCurrentTraceFrame();
    m_pMapWidget->reqUpdateCurrentTraceFrame();
}

void NaviWidget::updateFrontRadarInfoSlot(const FRONT_RADAR_INFO& info)
{
//    COM_DEBUG(5, ("ADC: Received front radar info: "));
//    for (int i = 0; i < FRONT_RADAR_INFO::MAX_OBJ_COUNT; ++i) {
//        COM_DEBUG(5, ("ADC: [%d] distance[%f] xLocation[%f] relSpeed[%f]", i,
//                      info.distance[i], info.xLocation[i], info.relSpeed[i]));
//    }

    m_pDrivingMapWidget->reqUpdateFrontRadarInfo(info);
}

void NaviWidget::updateBackRadarInfoSlot(const BACK_RADAR_INFO& info)
{
//    COM_DEBUG(5, ("ADC: Received back radar info: "));
//    for (int i = 0; i < BACK_RADAR_INFO::MAX_OBJ_COUNT; ++i) {
//        COM_DEBUG(5, ("ADC: [%d] leftDistance[%d] rightDistance[%d]", i,
//                      info.leftDistance[i], info.rightDistance[i]));
//    }

    m_pDrivingMapWidget->reqUpdateBackRadarInfo(info);
}

void NaviWidget::updateLidarInfoSlot(const IBEO_INFO& info)
{
    m_pDrivingMapWidget->reqUpdateLidarInfo(info);
}

void NaviWidget::onOpenCamCapture(void)
{
    switch (m_nPbOpenCamStatus) {
    case (PB_OPEN_CAM_STATUS_CLOSED):
        COM_DEBUG(5, ("ADC: NaviWidget::onOpenCamCapture, Open camera capture module"));
        if (m_pCamCaptureControl->StartCam()) {
            m_pOpenCamCaptureAction->setText(tr("Close Camera Capture"));
            m_nPbOpenCamStatus = PB_OPEN_CAM_STATUS_OPENED;
            onChangeToCamView();
            COM_DEBUG(5, ("ADC: NaviWidget::onOpenCamCapture, Open camera capture module OK"));
        } else {
            COM_ERR("Open camera capture failed");
            QMessageBox::warning(this,
                                 tr("Autonomous Driving Control"),
                                 tr("Open camera capture failed"),
                                 QMessageBox::Yes);
        }
        break;
    case (PB_OPEN_CAM_STATUS_OPENED):
        COM_DEBUG(5, ("ADC: NaviWidget::onOpenCamCapture, Close camera capture module"));
        if (m_pCamCaptureControl->EndCam()) {
            m_pOpenCamCaptureAction->setText(tr("Open Camera Capture"));
            m_nPbOpenCamStatus = PB_OPEN_CAM_STATUS_CLOSED;
            COM_DEBUG(5, ("ADC: NaviWidget::onOpenCamCapture, Close camera capture module OK"));
            onChangeToMapView();
        } else {
            COM_ERR("Close camera capture failed");
            QMessageBox::warning(this,
                                 tr("Autonomous Driving Control"),
                                 tr("Close camera capture failed"),
                                 QMessageBox::Yes);
        }
        break;
    default:
        break;
    }
}

void NaviWidget::onChangeToMapView(void)
{
    COM_DEBUG(5, ("ADC: change to map view"));
    if (VIEW_STATUS_MAP != m_nViewStatus) {
        m_nViewStatus = VIEW_STATUS_MAP;
        updateView();
        update();
    }
}

void NaviWidget::onChangeToCamView(void)
{
    COM_DEBUG(5, ("ADC: change to camera view"));
    if (VIEW_STATUS_CAMERA != m_nViewStatus) {
        m_nViewStatus = VIEW_STATUS_CAMERA;
        updateView();
        update();
    }
}

void NaviWidget::onCamRecoRetImgChanged(uchar* PictureData, int width, int height, int direction, float xDistance, float yDistance)
{
    m_pDrivingMapWidget->reqUpdateCamRecoRetImg(PictureData, width, height, direction, xDistance, yDistance);
}

void NaviWidget::onStartAutoDriving(void)
{
    switch (m_nPbAutoDrivingStatus) {
    case (PB_START_AUTO_DRIVING_STATUS_STOPPED):
        COM_DEBUG(5, ("ADC: NaviWidget::onStartAutoDriving, Start Auto Driving"));
        m_pDrivingMapWidget->enableAutoDriving(true);
        m_pPbStartAutoDriving->setText(tr("Stop"));
        onShowStatusInfo("Auto Driving started", 2);
        m_nPbAutoDrivingStatus = PB_START_AUTO_DRIVING_STATUS_STARTED;
        break;
    case (PB_START_AUTO_DRIVING_STATUS_STARTED):
        COM_DEBUG(5, ("ADC: NaviWidget::onStartAutoDriving, Stop Auto Driving"));
        m_pDrivingMapWidget->enableAutoDriving(false);
        m_pPbStartAutoDriving->setText(tr("Start"));
        onShowStatusInfo("Auto Driving stopped", 2);
        m_nPbAutoDrivingStatus = PB_START_AUTO_DRIVING_STATUS_STOPPED;
        break;
    default:
        break;
    }
}

void NaviWidget::onShowStatusInfo(const QString& info, int level)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(255,255,255));
    if (0 == level) {
        palette.setColor(QPalette::Background, QColor(255, 0, 0, 127));
    } else if ( 1 == level) {
        palette.setColor(QPalette::Background, QColor(255, 255, 0, 127));
    } else {
        //palette.setColor(QPalette::Background, QColor(0, 255, 0, 127));
        palette.setColor(QPalette::Background, QColor(82, 128, 233, 127));
    }
    m_pLabelShowStatusInfo->setPalette(palette);
    m_pLabelShowStatusInfo->setText(info);
}

NaviData& NaviWidget::getNaviData(void)
{
    return NaviData::getNaviData();
}

void NaviWidget::updateView(void)
{
    const static int interval = 2;
    const static int pbWidth = 120;
    const static int pbHeigh = 50;

    int extraWidth = width();
    int extraHeight = height();

    // 菜单按钮
    int menuBarPositionX = 0;
    int menuBarPosiztionY = 0;
    int menuBarWidth = m_pNaviMenuBar->width();
    int menuBarHeight = pbHeigh;
    //m_pNaviMenuBar->setGeometry(menuBarPositionX, menuBarPosiztionY, menuBarWidth, menuBarHeight);
    //m_pNaviMemu->setFixedSize(pbWidth, pbHeigh);
    // 摄像头按钮
    int camPbPositionX = menuBarPositionX + menuBarWidth + interval;
    int camPbPositionY = 0;
    int camPbWidth = pbWidth;
    int camPbHeight = pbHeigh;
    m_pPbChangeToCamView->setGeometry(camPbPositionX, camPbPositionY, camPbWidth, camPbHeight);
    // 地图按钮
    int mapPbPositionX = camPbPositionX + camPbWidth + interval;
    int mapPbPositionY = 0;
    int mapPbWidth = pbWidth;
    int mapPbHeight = pbHeigh;
    m_pPbChangeToMapView->setGeometry(mapPbPositionX, mapPbPositionY, mapPbWidth, mapPbHeight);
    // 开始自动驾驶按钮
    int startAutoDrivingPbPositionX = mapPbPositionX + mapPbWidth + interval;
    int startAutoDrivingPbPositionY = 0;
    int startAutoDrivingPbWidth = pbWidth;
    int startAutoDrivingPbHeight = pbHeigh;
    m_pPbStartAutoDriving->setGeometry(startAutoDrivingPbPositionX, startAutoDrivingPbPositionY,
                                       startAutoDrivingPbWidth, startAutoDrivingPbHeight);
    // 状态栏
    int showStatusInfoLabelPositionX = startAutoDrivingPbPositionX + startAutoDrivingPbWidth + interval;
    int showStatusInfoLabelPositionY = 0;
    int showStatusInfoLabelWidth = extraWidth - showStatusInfoLabelPositionX;
    int showStatusInfoLabelHeight = pbHeigh;
    m_pLabelShowStatusInfo->setGeometry(showStatusInfoLabelPositionX, showStatusInfoLabelPositionY,
                                       showStatusInfoLabelWidth, showStatusInfoLabelHeight);

    if (VIEW_STATUS_MAP == m_nViewStatus) {
        // 隐藏图片显示界面
        //m_pCamSignImgDisplayWidget->hide();
        //m_pCamLaneImgDisplayWidget->hide();
        m_pShowCamInfo->hide();
        // 显示地图界面
        m_pMapWidget->show();

        int mapPositionX = 0;
        int mapPositionY = pbHeigh + interval;
        int mapWidth = extraWidth / 2 - interval / 2;
        int mapHeight = extraHeight - mapPositionY;
        m_pMapWidget->setGeometry(mapPositionX, mapPositionY, mapWidth, mapHeight);

        int drivingMapPositionX = mapWidth + interval;
        int drivingMapPositionY = pbHeigh + interval;
        int drivingMapWidth = extraWidth - mapWidth - interval;
        int drivingMapHeight = extraHeight - mapPositionY;
        m_pDrivingMapWidget->setGeometry(drivingMapPositionX, drivingMapPositionY, drivingMapWidth, drivingMapHeight);
    } else {
        // 隐藏地图界面
        m_pMapWidget->hide();
        // 显示camera界面
        //m_pCamSignImgDisplayWidget->show();
        //m_pCamLaneImgDisplayWidget->show();
        m_pShowCamInfo->show();

//        int camSignWidgetPosX = 0;
//        int camSignWidgetPosY = pbHeigh + interval;
//        int camSignWidgetWidth = extraWidth / 2;
//        int camSignWidgetHeight = extraHeight / 2 - camSignWidgetPosY;
//        m_pCamSignImgDisplayWidget->setGeometry(camSignWidgetPosX, camSignWidgetPosY, camSignWidgetWidth, camSignWidgetHeight);

//        int camLaneWidgetPosX = 0;
//        int camLaneWidgetPosY = camSignWidgetPosY + camSignWidgetHeight;
//        int camLaneWidgetWidth = extraWidth / 2;
//        int camLaneWidgetHeight = camSignWidgetHeight;
//        m_pCamLaneImgDisplayWidget->setGeometry(camLaneWidgetPosX, camLaneWidgetPosY, camLaneWidgetWidth, camLaneWidgetHeight);

//        int drivingMapPositionX = camSignWidgetWidth + interval;
//        int drivingMapPositionY = pbHeigh + interval;
//        int drivingMapWidth = extraWidth - camSignWidgetWidth - interval / 2;
//        int drivingMapHeight = extraHeight  - drivingMapPositionY;
//        m_pDrivingMapWidget->setGeometry(drivingMapPositionX, drivingMapPositionY, drivingMapWidth, drivingMapHeight);
        int camFramePositionX = 0;
        int camFramePositionY = pbHeigh + interval;
        int camFrameWidth = 480;
        int camFrameHeight = extraHeight - camFramePositionY;
        m_pShowCamInfo->setGeometry(camFramePositionX, camFramePositionY, camFrameWidth, camFrameHeight);

        int drivingMapPositionX = camFrameWidth + interval;
        int drivingMapPositionY = pbHeigh + interval;
        int drivingMapWidth = extraWidth - camFrameWidth;
        int drivingMapHeight = extraHeight - camFramePositionY;
        m_pDrivingMapWidget->setGeometry(drivingMapPositionX, drivingMapPositionY, drivingMapWidth, drivingMapHeight);
    }
}

