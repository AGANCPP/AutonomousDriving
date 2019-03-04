#ifndef NAVIDRIVINGWIDGET_H
#define NAVIDRIVINGWIDGET_H

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QtOpenGL/QGLWidget>

#include "navidata.h"
#include "IHighAccuracyDataReader.h"
#include "radardata.h"
#include "movementcontrol.h"

enum {
    GPS_STATUS_INVALID,
    GPS_STATUS_OK
};

/******************************************************************************
 * class define
 ******************************************************************************/
class NaviData;
class DrivingMapWidget : public QGLWidget
{
    Q_OBJECT

public:
    DrivingMapWidget(NaviData *pNaviData, QWidget* parent = 0);
    ~DrivingMapWidget();

    void setNaviData(NaviData* pNaviData) { m_pNaviData = pNaviData; }
    void reqUpdateMapFrame(void);
    void reqUpdateHistoryTraceFrame(void);
    void reqUpdateCurrentTraceFrame(void);
    void reqUpdateFrontRadarInfo(const FRONT_RADAR_INFO& info);
    void reqUpdateBackRadarInfo(const BACK_RADAR_INFO& info);
    void reqUpdateLidarInfo(const IBEO_INFO& info);
    void reqUpdateCamRecoRetImg(uchar* PictureData, int width, int height, int direction, float xDistance, float yDistance);

    void enableAutoDriving(bool enable)
    {
        m_bEnableAutoDrivingFlag = enable;
        if (false == enable) {
            m_movementCtl.reset();
        }
    }

signals:
    void showInfoToStatusBar(const QString& info, int level);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void timerEvent(QTimerEvent *event);

private:
    void reshape();
    void calculateDefaultViewArea();
    bool isClipedScreen(MAP_RECTANGLE& rect);   // 判断场景中的图元是否在视口内
    void recoverDefaultViewWindow();            // 恢复默认的显示窗口
    void updateViewWindow();

    void paintMap();
    void paintCurrentTrace();
    void paintCar();
    void paintObstacles();
    void paintDrivingInfo();
    void paintCamInfo();
    void paintMovementCtlInfo();
    void drawMap();
    void draw3DHighAccuracyData();
    void drawLaneSection(const KHI_LANE_SECTION *pSection);
    void drawRoadCross(const KHI_ROAD_CROSS* pCross);
    void drawRoadLaneLineHAD();
    void drawRoadAreaHAD();
    void drawPaintMark();
    void drawHistoryTrace();
    void drawCurrentTrace();
    void drawCar();
    void drawObstacles();
    void drawDrivingInfo();
    void drawCamInfo();
    void drawMovementCtlInfo();

    void drawDetectZone(POINT_2D_ origin);
    void drawDetailZone(POINT_2D_ origin,
                        double sDis,double wDis,     //sDis-探测距离 wDis-危险距离
                        float sAngle,float nAngle);  //sAngle-法线角度 nAngle-探测角度

    void drawPoint(double x,double y,double z,double r);
    void drawCircle(double x,double y,double z,double r,unsigned int type);

    void drawRect(double x,double y,double z,double w,double h);

    void loadCarIcon();
    void clearRadarInfo();

    void loadBgImg();
    void drawBgImg();
    void paintBgImg();

private:
    // 定时刷新timer(QObject 定时器)
    int m_nWorkTimer;

    // 视野 (米单位)
    double m_carLeftFieldOfVisionMetre;
    double m_carRightFieldOfVisionMetre;
    double m_carFrontFieldOfVisionMetre;
    double m_carBackFieldOfVisionMetre;
    // 视野 (GPS 坐标单位)
    double m_carLeftFieldOfVision;
    double m_carRightFieldOfVision;
    double m_carFrontFieldOfVision;
    double m_carBackFieldOfVision;
    // 车身尺寸 (米单位)
    double m_carWidthMetre;
    double m_carHeightMetre;
    // 车身尺寸 (GPS 坐标单位)
    double m_halfCarWidth;
    double m_halfCarHeight;
    GLuint m_carIconTextureId;
    GLuint m_naviDrvingImgTextureId;

    // 导航相关数据
    NaviData* m_pNaviData;
    // 用于地图缩放
    GLfloat m_fScaling;                         // 场景放大比例
    float m_fMapScale;                          // 场景与屏幕的比例
    // 用于地图移动
    QPoint m_lastPos;                           // 记录鼠标的位置
    GLdouble m_moveDx;                          // 记录移动的位置
    GLdouble m_moveDy;                          // 记录移动的位置
    // 用于控制显示地图的窗口
    MAP_RECTANGLE m_defaultViewArea;            // 默认视口
    MAP_RECTANGLE m_viewWindow;                 // 记录视口的窗口在场景中的位置
    enum { MAX_POINT_BUFF = 30720 };
    MAP_POINT m_tempPointBuff[MAX_POINT_BUFF];  // 临时保存转换后的坐标
    // 判断当前位置变化了吗
    bool m_bCurrentPositionChanged;
    // GPS
    int m_gpsStatus;
    GPS_INFO m_currentGps;
    MAP_POINT m_currentPoint;
    double m_currentLattitude;                 // 纬度
    double m_currentLongitude;                 // 经度

    // 驾驶信息
    typedef struct DRIVING_INFO_ {
        //
        KHI_LANE_SECTION* pLaneSection;
        MAP_POINT nearestPoint;
        double nearestDistance;
    }DRIVING_INFO;
    DRIVING_INFO m_drivingInfo;

    // 轨迹控制
    MovementControl m_movementCtl;
    // 启动自动驾驶
    volatile bool m_bEnableAutoDrivingFlag;

    // 雷达数据
    typedef struct RADAR_INFO_ {
        struct {
            float distance;
            float xLocation;
            float relSpeed;
            MAP_POINT objPoint;
        }frontRadar[FRONT_RADAR_INFO::MAX_OBJ_COUNT];
        struct {
            float distance;
            MAP_POINT objPoint;
        }backLeftRadar[BACK_RADAR_INFO::MAX_OBJ_COUNT],
        backRightRadar[BACK_RADAR_INFO::MAX_OBJ_COUNT];
    }RADAR_INFO;
    RADAR_INFO m_radarInfo;
    // 激光雷达数据
    typedef struct LIDAR_INFO_ {
        struct {
            double distance;
            IBEO_OBJECT_INFO objInfo;
            MAP_POINT objPoint;
            MAP_POINT boundingCenterPoint;
        }frontLidar[IBEO_INFO::MAX_OBJECTS];
    }LIDAR_INFO;
    LIDAR_INFO m_lidarInfo;

    typedef struct CAM_INFO_ {
        int direction;
        float xDistance;
        float yDistance;
        MAP_POINT point;
        GLuint m_recoRetImgTextureId;
        struct {
            MAP_POINT leftBottom;
            MAP_POINT rightBottom;
            MAP_POINT rightTop;
            MAP_POINT leftTop;
        }imgPos;
        CAM_INFO_()
        {
            direction = 0;
            xDistance = 0.0;
            yDistance = 0.0;
            m_recoRetImgTextureId = 0;
        }
    }CAM_INFO;
    CAM_INFO m_camInfo;
};

#endif // NAVIDRIVINGWIDGET_H
