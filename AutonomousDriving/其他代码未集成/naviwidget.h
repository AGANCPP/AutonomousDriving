#ifndef NAVIWIDGET_H
#define NAVIWIDGET_H

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QWidget>
#include <QtOpenGL/QGLWidget>

////#include "navidata.h"
////#include "radardata.h"
////#include "camControl.h"
#include "showcaminfo.h"

/******************************************************************************
 * class define
 ******************************************************************************/
class QMenuBar;
class QMenu;
class QAction;
class QPushButton;
class NaviData;
class MapWidget;
class DrivingMapWidget;

class NaviWidget : public QWidget
{
    Q_OBJECT

public:
    NaviWidget(QWidget *parent = 0);
    ~NaviWidget();

public slots:
    void dummySlot(void);
    void loadMapFileSlot(void);
    void loadHistoryTraceSlot(void);
    void updateCurrentTraceSlot(const GPS_INFO& position);
    void updateFrontRadarInfoSlot(const FRONT_RADAR_INFO& info);
    void updateBackRadarInfoSlot(const BACK_RADAR_INFO& info);
    void updateLidarInfoSlot(const IBEO_INFO& info);
    void onOpenCamCapture(void);
    void onChangeToMapView(void);
    void onChangeToCamView(void);
    void onCamRecoRetImgChanged(uchar* PictureData, int width, int height, int direction, float xDistance, float yDistance);
    void onStartAutoDriving(void);
    void onShowStatusInfo(const QString& info, int level);

protected:
    void resizeEvent(QResizeEvent *);

private:
    NaviData& getNaviData(void);
    void updateView(void);

private:
    QMenuBar* m_pNaviMenuBar;
    QMenu* m_pNaviMemu;
    QAction* m_pLoadMapAction;
    QAction* m_pLoadHistoryTraceAction;
    QAction* m_pOpenCamCaptureAction;
    enum {
        PB_OPEN_CAM_STATUS_CLOSED,
        PB_OPEN_CAM_STATUS_OPENED
    };
    int m_nPbOpenCamStatus;
    // 视图切换按钮
    QPushButton* m_pPbChangeToMapView;
    QPushButton* m_pPbChangeToCamView;
    enum {
        VIEW_STATUS_MAP,
        VIEW_STATUS_CAMERA
    };
    int m_nViewStatus;
    // 启动自动控制按钮
    QPushButton* m_pPbStartAutoDriving;
    enum {
        PB_START_AUTO_DRIVING_STATUS_STOPPED,
        PB_START_AUTO_DRIVING_STATUS_STARTED
    };
    int m_nPbAutoDrivingStatus;
    // 状态栏
    QLabel* m_pLabelShowStatusInfo;

    MapWidget* m_pMapWidget;
    DrivingMapWidget* m_pDrivingMapWidget;
    CamControl* m_pCamCaptureControl;
    //ImgDisplayWidget* m_pCamSignImgDisplayWidget;
    //ImgDisplayWidget* m_pCamLaneImgDisplayWidget;
    ShowCamInfo* m_pShowCamInfo;
};

#endif // NAVIWIDGET_H
