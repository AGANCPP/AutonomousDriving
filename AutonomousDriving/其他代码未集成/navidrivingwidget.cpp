// navidrivingwidget.cpp

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QtGui>
#include <QGLFramebufferObject>

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string>

#include "navidrivingwidget.h"
#include "commondebug.h"
#include "commontools.h"

/******************************************************************************
 * class
 ******************************************************************************/
DrivingMapWidget::DrivingMapWidget(NaviData *pNaviData, QWidget* parent) :
    QGLWidget(parent),
    m_movementCtl(pNaviData)
{
    // 地图相关变量初始化
    m_pNaviData = pNaviData;

    m_fMapScale = 1.0;
    m_fScaling = 1.0;

    m_moveDx = 0.0;
    m_moveDy = 0.0;

    m_viewWindow.left = 0;
    m_viewWindow.bottom = 0;
    m_viewWindow.right = width();
    m_viewWindow.top = height();

    m_gpsStatus = GPS_STATUS_INVALID;
    m_bCurrentPositionChanged = false;
    m_currentLattitude = 0.0;                 // 纬度
    m_currentLongitude = 0.0;                 // 经度

    memset(&m_currentGps, 0, sizeof(m_currentGps));
    memset(&m_currentPoint, 0, sizeof(m_currentPoint));
    //m_nWorkTimer = 0;

    // 视野 (米单位)
    m_carLeftFieldOfVisionMetre = 30.0;
    m_carRightFieldOfVisionMetre = 30.0;
    m_carFrontFieldOfVisionMetre = 100.0;
    m_carBackFieldOfVisionMetre = 80.0;
    // 视野 (GPS 坐标单位)
    m_carLeftFieldOfVision = meterToGeoCoord(m_carLeftFieldOfVisionMetre);
    m_carRightFieldOfVision = meterToGeoCoord(m_carRightFieldOfVisionMetre);
    m_carFrontFieldOfVision = meterToGeoCoord(m_carFrontFieldOfVisionMetre);
    m_carBackFieldOfVision = meterToGeoCoord(m_carBackFieldOfVisionMetre);

    m_carWidthMetre = 1.72;
    m_carHeightMetre = 4.68;
    m_halfCarWidth = meterToGeoCoord(m_carWidthMetre) / 2.0;
    m_halfCarHeight = meterToGeoCoord(m_carHeightMetre) / 2.0;
    m_carIconTextureId = 0;
    m_naviDrvingImgTextureId = 0;

    memset(&m_drivingInfo, 0, sizeof(m_drivingInfo));
    m_drivingInfo.nearestDistance = 999999999.0;

    COM_DEBUG(5, ("ADC: Half Car Width (%f), Half Car Height (%f)", m_halfCarWidth, m_halfCarHeight));

    clearRadarInfo();

    calculateDefaultViewArea();

    makeCurrent();

    loadCarIcon();
    loadBgImg();

    m_bEnableAutoDrivingFlag = false;

    // 开启定时工作timer（周期性处理GPS坐标和障碍物）
    m_nWorkTimer = this->startTimer(80);
    if (0 == m_nWorkTimer) {
        COM_ERR("[ERR]ADC: DrivingMapWidget, start Timer error");
    }
}

DrivingMapWidget::~DrivingMapWidget()
{
    makeCurrent();

    if (0 != m_nWorkTimer) {
        killTimer(m_nWorkTimer);
    }
}

void DrivingMapWidget::reqUpdateMapFrame(void)
{
    calculateDefaultViewArea();
    // Draw map
    reshape();
    update();
}

void DrivingMapWidget::reqUpdateHistoryTraceFrame(void)
{
    m_movementCtl.reqUpdateHistoryTrace();

    calculateDefaultViewArea();
    // Draw map
    reshape();
    update();
}

void DrivingMapWidget::reqUpdateCurrentTraceFrame(void)
{
    // Draw map
    reshape();
    //update();
}

void DrivingMapWidget::reqUpdateFrontRadarInfo(const FRONT_RADAR_INFO& info)
{
    POINT_2D point;
    POINT_2D pointRotateCenter;
    Matrix3x3 rotMat;

    // 构造旋转矩阵
    matrix3x3SetIdentity(rotMat);
    pointRotateCenter.x = m_currentPoint.x;
    pointRotateCenter.y = m_currentPoint.y;
    rotate2D(pointRotateCenter, ((360.0 - m_currentGps.Heading) * PI) / 180.0, rotMat);

    int nObjectCount = 0;
    for (int i = 0; i < FRONT_RADAR_INFO::MAX_OBJ_COUNT; ++i) {
        if (info.distance[i] > 0.001) {
            m_radarInfo.frontRadar[i].distance = info.distance[i];
            m_radarInfo.frontRadar[i].xLocation = info.xLocation[i];
            m_radarInfo.frontRadar[i].relSpeed = info.relSpeed[i];

            double dy = sqrt(m_radarInfo.frontRadar[i].distance * m_radarInfo.frontRadar[i].distance -
                      m_radarInfo.frontRadar[i].xLocation * m_radarInfo.frontRadar[i].xLocation);

            point.x = m_currentPoint.x + meterToGeoCoord(m_radarInfo.frontRadar[i].xLocation);
            point.y = m_currentPoint.y + (meterToGeoCoord(dy) + m_halfCarHeight);
            transformVerts2D(rotMat, 1, &point);
            m_radarInfo.frontRadar[i].objPoint.x = point.x;
            m_radarInfo.frontRadar[i].objPoint.y = point.y;

            nObjectCount++;
        } else {
            m_radarInfo.frontRadar[i].distance = 0;
            m_radarInfo.frontRadar[i].xLocation = 0;
            m_radarInfo.frontRadar[i].relSpeed = 0;
            m_radarInfo.frontRadar[i].objPoint.x = 0;
            m_radarInfo.frontRadar[i].objPoint.y = 0;
        }
    }

//    if (nObjectCount > 0) {
//        update();
//    }

    //COM_DEBUG(5, ("DrivingMapWidget::reqUpdateFrontRadarInfo, objects[%d]", nObjectCount));
}

void DrivingMapWidget::reqUpdateBackRadarInfo(const BACK_RADAR_INFO& info)
{
    const static double dxScale = cos((70.0 * PI) / 180.0);
    const static double dyScale = sin((70.0 * PI) / 180.0);

    POINT_2D point;
    POINT_2D pointRotateCenter;
    Matrix3x3 rotMat;

    // 构造旋转矩阵
    matrix3x3SetIdentity(rotMat);
    pointRotateCenter.x = m_currentPoint.x;
    pointRotateCenter.y = m_currentPoint.y;
    rotate2D(pointRotateCenter, ((360.0 - m_currentGps.Heading) * PI) / 180.0, rotMat);

    int nObjectCount = 0;

    for (int i = 0; i < BACK_RADAR_INFO::MAX_OBJ_COUNT; ++i) {
        //
        if (info.leftDistance[i] > 0) {
            m_radarInfo.backLeftRadar[i].distance = info.leftDistance[i] / 10.0;    // 分米 -> 米
            point.x = m_currentPoint.x - meterToGeoCoord(m_radarInfo.backLeftRadar[i].distance * dxScale) - m_halfCarWidth;
            point.y = m_currentPoint.y - (meterToGeoCoord(m_radarInfo.backLeftRadar[i].distance * dyScale) + m_halfCarHeight);
            transformVerts2D(rotMat, 1, &point);
            m_radarInfo.backLeftRadar[i].objPoint.x = point.x;
            m_radarInfo.backLeftRadar[i].objPoint.y = point.y;

            nObjectCount++;
        } else {
            m_radarInfo.backLeftRadar[i].distance = 0;
            m_radarInfo.backLeftRadar[i].objPoint.x = 0;
            m_radarInfo.backLeftRadar[i].objPoint.y = 0;
        }

        if (info.rightDistance[i] > 0) {
            m_radarInfo.backRightRadar[i].distance = info.rightDistance[i] / 10.0;
            point.x = m_currentPoint.x + meterToGeoCoord(m_radarInfo.backRightRadar[i].distance * dxScale) + m_halfCarWidth;
            point.y = m_currentPoint.y - (meterToGeoCoord(m_radarInfo.backRightRadar[i].distance * dyScale) + m_halfCarHeight);
            transformVerts2D(rotMat, 1, &point);
            m_radarInfo.backRightRadar[i].objPoint.x = point.x;
            m_radarInfo.backRightRadar[i].objPoint.y = point.y;
            nObjectCount++;
        } else {
            m_radarInfo.backRightRadar[i].distance = 0;
            m_radarInfo.backRightRadar[i].objPoint.x = 0;
            m_radarInfo.backRightRadar[i].objPoint.y = 0;
        }
    }

//    if (nObjectCount > 0) {
//        update();
//    }

    //COM_DEBUG(5, ("DrivingMapWidget::reqUpdateBackRadarInfo, objects[%d]", nObjectCount));
}

void DrivingMapWidget::reqUpdateLidarInfo(const IBEO_INFO& info)
{
    POINT_2D point;
    POINT_2D boundingCenterPoint;
    POINT_2D pointRotateCenter;
    Matrix3x3 rotMat;

    // 构造旋转矩阵
    matrix3x3SetIdentity(rotMat);
    pointRotateCenter.x = m_currentPoint.x;
    pointRotateCenter.y = m_currentPoint.y;
    rotate2D(pointRotateCenter, ((360.0 - m_currentGps.Heading) * PI) / 180.0, rotMat);

    int nObjectCount = 0;
    for (int i = 0; i < IBEO_INFO::MAX_OBJECTS; ++i) {
        if (1 == info.ibeoObjects[i].flag) {
            m_lidarInfo.frontLidar[i].objInfo.flag = info.ibeoObjects[i].flag;
            m_lidarInfo.frontLidar[i].objInfo.objectID = info.ibeoObjects[i].objectID;
            m_lidarInfo.frontLidar[i].objInfo.classification = info.ibeoObjects[i].classification;
            m_lidarInfo.frontLidar[i].objInfo.classificationAge = info.ibeoObjects[i].classificationAge;
            m_lidarInfo.frontLidar[i].objInfo.centerpt_x = info.ibeoObjects[i].centerpt_x;
            m_lidarInfo.frontLidar[i].objInfo.centerpt_y = info.ibeoObjects[i].centerpt_y;
            m_lidarInfo.frontLidar[i].objInfo.boundingbox_x = info.ibeoObjects[i].boundingbox_x;
            m_lidarInfo.frontLidar[i].objInfo.boundingbox_y = info.ibeoObjects[i].boundingbox_y;
            m_lidarInfo.frontLidar[i].objInfo.boundingCenter_x = info.ibeoObjects[i].boundingCenter_x;
            m_lidarInfo.frontLidar[i].objInfo.boundingCenter_y = info.ibeoObjects[i].boundingCenter_y;

//            m_lidarInfo.frontLidar[i].distance = sqrt((info.ibeoObjects[i].centerpt_x * info.ibeoObjects[i].centerpt_x) +
//                                                      (info.ibeoObjects[i].centerpt_y * info.ibeoObjects[i].centerpt_y));
            m_lidarInfo.frontLidar[i].distance = sqrt((info.ibeoObjects[i].boundingCenter_x * info.ibeoObjects[i].boundingCenter_x) +
                                                      (info.ibeoObjects[i].boundingCenter_y * info.ibeoObjects[i].boundingCenter_y));

            point.x = m_currentPoint.x - meterToGeoCoord(m_lidarInfo.frontLidar[i].objInfo.centerpt_y);
            point.y = m_currentPoint.y + meterToGeoCoord(m_lidarInfo.frontLidar[i].objInfo.centerpt_x) + m_halfCarHeight;
            transformVerts2D(rotMat, 1, &point);
            m_lidarInfo.frontLidar[i].objPoint.x = point.x;
            m_lidarInfo.frontLidar[i].objPoint.y = point.y;
            boundingCenterPoint.x = m_currentPoint.x - meterToGeoCoord(m_lidarInfo.frontLidar[i].objInfo.boundingCenter_y);
            boundingCenterPoint.y = m_currentPoint.y + meterToGeoCoord(m_lidarInfo.frontLidar[i].objInfo.boundingCenter_x) + m_halfCarHeight;
            transformVerts2D(rotMat, 1, &boundingCenterPoint);
            m_lidarInfo.frontLidar[i].boundingCenterPoint.x = boundingCenterPoint.x;
            m_lidarInfo.frontLidar[i].boundingCenterPoint.y = boundingCenterPoint.y;

            nObjectCount++;
        } else {
            m_lidarInfo.frontLidar[i].objInfo.flag = 0;
            m_lidarInfo.frontLidar[i].objInfo.objectID = 0;
            m_lidarInfo.frontLidar[i].objInfo.classification = 0;
            m_lidarInfo.frontLidar[i].objInfo.classificationAge = 0;
            m_lidarInfo.frontLidar[i].objInfo.centerpt_x = 0;
            m_lidarInfo.frontLidar[i].objInfo.centerpt_y = 0;
            m_lidarInfo.frontLidar[i].objInfo.boundingbox_x = 0;
            m_lidarInfo.frontLidar[i].objInfo.boundingbox_y = 0;
            m_lidarInfo.frontLidar[i].objInfo.boundingCenter_x = 0;
            m_lidarInfo.frontLidar[i].objInfo.boundingCenter_y = 0;

            m_lidarInfo.frontLidar[i].distance = 0;

            m_lidarInfo.frontLidar[i].objPoint.x = 0;
            m_lidarInfo.frontLidar[i].objPoint.y = 0;
            m_lidarInfo.frontLidar[i].boundingCenterPoint.x = 0;
            m_lidarInfo.frontLidar[i].boundingCenterPoint.y = 0;
        }
    }
}

void DrivingMapWidget::reqUpdateCamRecoRetImg(uchar* PictureData, int width, int height, int direction, float xDistance, float yDistance)
{
    //COM_DEBUG(5, ("ImgDisplayWidget::onShowScreen, width[%d] height[%d]", width, height));
    QImage* img2 = new QImage(PictureData, width, height, width*3, QImage::Format_RGB888);
    QImage img = img2->scaled(64, 64);
    delete img2;

    if (m_camInfo.m_recoRetImgTextureId) {
        glDeleteTextures(1, &m_camInfo.m_recoRetImgTextureId);
    }

    GLuint idTexture = 0;

    glGenTextures(1, &idTexture);
    if (idTexture)
    {
        glBindTexture(GL_TEXTURE_2D, idTexture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // 生成纹理
        glTexImage2D(GL_TEXTURE_2D,0,3,img.width(), img.height(),0,GL_RGB,GL_UNSIGNED_BYTE,img.bits());
    }

    // 计算坐标
    MAP_POINT point;
    POINT_2D pointRotateCenter;
    Matrix3x3 rotMat;
    // 构造旋转矩阵
    matrix3x3SetIdentity(rotMat);
    pointRotateCenter.x = m_currentPoint.x;
    pointRotateCenter.y = m_currentPoint.y;
    rotate2D(pointRotateCenter, ((-m_currentGps.Heading) * PI) / 180.0, rotMat);
    if (1 == direction) {
        // 左边
        point.x = m_currentPoint.x - meterToGeoCoord(yDistance / 1000.0) - m_halfCarWidth;
    } else {
        // 右边
        point.x = m_currentPoint.x + meterToGeoCoord(yDistance / 1000.0) + m_halfCarWidth;
    }
    point.y = m_currentPoint.y + meterToGeoCoord(xDistance / 1000.0) + m_halfCarHeight;

    static double signWidth = 200;
    static double signHeight = 200;
    if (1 == m_camInfo.direction) {
        // 左边
        // 左下
        m_camInfo.imgPos.leftBottom.x = point.x - signWidth;
        m_camInfo.imgPos.leftBottom.y = point.y;
        // 右下
        m_camInfo.imgPos.rightBottom.x = point.x;
        m_camInfo.imgPos.rightBottom.y = point.y;
        // 右上
        m_camInfo.imgPos.rightTop.x = point.x;
        m_camInfo.imgPos.rightTop.y = point.y + signHeight;
        // 左上
        m_camInfo.imgPos.leftTop.x = point.x - signHeight;
        m_camInfo.imgPos.leftTop.y = point.y + signHeight;
    } else {
        // 右边
        // 左下
        m_camInfo.imgPos.leftBottom.x = point.x;
        m_camInfo.imgPos.leftBottom.y = point.y;
        // 右下
        m_camInfo.imgPos.rightBottom.x = point.x + signWidth;
        m_camInfo.imgPos.rightBottom.y = point.y;
        // 右上
        m_camInfo.imgPos.rightTop.x = point.x + signWidth;
        m_camInfo.imgPos.rightTop.y = point.y + signHeight;
        // 左上
        m_camInfo.imgPos.leftTop.x = point.x;
        m_camInfo.imgPos.leftTop.y = point.y + signHeight;
    }
    transformVerts2D(rotMat, 1, &point);
    transformVerts2D(rotMat, 1, &m_camInfo.imgPos.leftBottom);
    transformVerts2D(rotMat, 1, &m_camInfo.imgPos.rightBottom);
    transformVerts2D(rotMat, 1, &m_camInfo.imgPos.rightTop);
    transformVerts2D(rotMat, 1, &m_camInfo.imgPos.leftTop);

    m_camInfo.m_recoRetImgTextureId = idTexture;
    m_camInfo.direction = direction;
    m_camInfo.xDistance = xDistance;
    m_camInfo.yDistance = yDistance;
    m_camInfo.point = point;
}

void DrivingMapWidget::initializeGL()
{
    qglClearColor(QColor(210, 210, 210, 0));
    glShadeModel(GL_FLAT);

    //开启抗锯齿
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DrivingMapWidget::resizeGL(int w, int h)
{
    reshape();
}

void DrivingMapWidget::reshape()
{
    // 更新显示区域
    updateViewWindow();
}

void DrivingMapWidget::paintGL()
{
//    unsigned int nStartTime = GetCpuTick();
//    unsigned int nEndTime = 0;


    // 清除当前场景
    glClear(GL_COLOR_BUFFER_BIT);

    // 定义视口
    glViewport(0, 0, width(), height());
    // 投影变换
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 定义正投影(裁剪区域)
    //gluOrtho2D(/*left*/, /*right*/, /*bottom*/, /*top*/);
    gluOrtho2D(0, width(), 0, height());
    // 在执行模型或视图变换之前，必须以GL_MODELVIEW为参数调用glMatrixMode()函数
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    paintBgImg();

    // 定义视口
    glViewport(0, 0, width(), height());
    // 投影变换
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 定义正投影(裁剪区域)
    //gluOrtho2D(/*left*/, /*right*/, /*bottom*/, /*top*/);
    gluOrtho2D(0, m_viewWindow.right - m_viewWindow.left, 0, m_viewWindow.top - m_viewWindow.bottom);
    // 在执行模型或视图变换之前，必须以GL_MODELVIEW为参数调用glMatrixMode()函数
    glMatrixMode(GL_MODELVIEW);

    // 旋转
    glLoadIdentity();
    double moveX = m_currentPoint.x - m_viewWindow.left;
    double moveY = m_currentPoint.y - m_viewWindow.bottom;
    glTranslated(moveX, moveY, 0);
    glRotated(m_currentGps.Heading, 0, 0, 1.0);
    glTranslated(-moveX, -moveY, 0);
    //qDebug("m_currentGps.Heading %f", m_currentGps.Heading);

    // 初始化驾驶信息
    m_drivingInfo.nearestDistance = 999999999.0;
    m_drivingInfo.nearestPoint.x = 0;
    m_drivingInfo.nearestPoint.y = 0;

    // 绘制地图
    paintMap();
    // 绘制当前轨迹
    paintCurrentTrace();
    // 绘制障碍物
    paintObstacles();
    // 绘制驾驶信息
    paintDrivingInfo();
    // 绘制相机识别结果
    paintCamInfo();

    // 控制
    if (m_bEnableAutoDrivingFlag) {
        int ret = m_movementCtl.refresh();
        if (MovementControl::CTL_OK != ret) {
            switch (ret) {
            case (MovementControl::CTL_END):
                emit showInfoToStatusBar("AutoDriving: Reached the end", 1);
                break;
            case (MovementControl::CTL_ERR_NO_GPS):
                emit showInfoToStatusBar("AutoDriving: No gps signal", 0);
                break;
            case (MovementControl::CTL_ERR_NO_TRACE):
                emit showInfoToStatusBar("AutoDriving: No trace infomation", 0);
                break;
            case (MovementControl::CTL_ERR_NOT_IN_RANGE):
                emit showInfoToStatusBar("AutoDriving: Not in range", 0);
                break;
            case (MovementControl::CTL_ERR_COMMUNICATION_FAILED):
                emit showInfoToStatusBar("AutoDriving: Communication failed", 0);
                break;
            default:
                break;
            }
        } else {
            emit showInfoToStatusBar("AutoDriving: Normal running", 2);
            paintMovementCtlInfo();
        }
    }

    // 车身不旋转
    glLoadIdentity();
    // 绘制车身
    paintCar();

//    nEndTime = GetCpuTick();
//    COM_DEBUG(5, ("ADC: paintGL expend %u ms", CalcElapsedTime(nStartTime, nEndTime)));
}

void DrivingMapWidget::timerEvent(QTimerEvent *event)
{
    //
    if (event->timerId() == m_nWorkTimer) {
        update();
    }
}

void DrivingMapWidget::calculateDefaultViewArea()
{
    std::vector<MAP_RECTANGLE> areas;

    if (m_pNaviData->m_mapData.isMapValid()) {
        //m_pNaviData->m_mapData.calculateMapArea();
        areas.push_back(m_pNaviData->m_mapData.m_mapArea);
    }

    if (m_pNaviData->m_historyTrace.isValid()) {
         //m_pNaviData->m_historyTrace.calculateArea();
         areas.push_back(m_pNaviData->m_historyTrace.m_area);
    }

    constructBoundingBox(areas, m_defaultViewArea);
    if ((m_defaultViewArea.right - m_defaultViewArea.left) < width()) {
        m_defaultViewArea.right = m_defaultViewArea.left + width();
        m_defaultViewArea.top = m_defaultViewArea.bottom + height();
    }

    COM_DEBUG(5, ("m_defaultViewArea[(%f, %f) (%f, %f)]",
           m_defaultViewArea.left, m_defaultViewArea.bottom, m_defaultViewArea.right, m_defaultViewArea.top));
}

void DrivingMapWidget::updateViewWindow()
{
#if 0
    GLdouble viewLeft = 0;
    GLdouble viewBottom = 0;
    GLdouble viewRight = width();
    GLdouble viewTop = height();

    // 根据地图大小，控制显示区域
    if (m_defaultViewArea.isValid()) {
        viewLeft = m_defaultViewArea.left;
        viewRight = m_defaultViewArea.right;
        viewBottom = m_defaultViewArea.bottom;
        viewTop = m_defaultViewArea.top;
    }

    // 限制缩放比例
    if (m_fScaling < 0.01) {
        m_fScaling = 0.01;
    } else if (m_fScaling > 10) {
        m_fScaling = 10;
    }

    // 计算显示区域
    GLdouble sizeX = viewRight - viewLeft;
    GLdouble sizeY = sizeX * ((GLdouble)height() / (GLdouble)width());
    GLdouble scaleX = sizeX * m_fScaling;
    GLdouble scaleY = sizeY * m_fScaling;
    m_viewWindow.left = ((viewLeft + viewRight - scaleX) / 2.0 + m_moveDx);
    m_viewWindow.bottom = ((viewBottom + viewTop - scaleY) / 2.0 + m_moveDy);
    m_viewWindow.right = ((viewLeft + viewRight + scaleX) / 2.0 + m_moveDx);
    m_viewWindow.top = ((viewBottom + viewTop + scaleY) / 2.0 + m_moveDy);
    // 计算地图显示比例尺
    m_fMapScale = (m_viewWindow.right - m_viewWindow.left) / width();
#else
    GLdouble viewLeft = 0;
    GLdouble viewBottom = 0;
    GLdouble viewRight = width();
    GLdouble viewTop = height();

    m_bCurrentPositionChanged = false;
    int size = m_pNaviData->m_currentTrace.m_currentTrace.size();
    if (size > 0) {
        GPS_INFO& current = m_pNaviData->m_currentTrace.m_currentTrace[size-1];
        m_currentGps = current;
        m_gpsStatus = GPS_STATUS_OK;
        if ((current.Lattitude != m_currentLattitude) ||
            (current.Longitude != m_currentLongitude)) {
            m_currentLattitude = current.Lattitude;
            m_currentLongitude = current.Longitude;
            m_bCurrentPositionChanged = true;

            convertPointUnit(current.Longitude, current.Lattitude, m_currentPoint);
        }
    } else {
        // no gps signal
        m_gpsStatus = GPS_STATUS_INVALID;
    }

//    viewLeft = m_currentPoint.x - 800;
//    viewRight = m_currentPoint.x + 800;
//    viewBottom = m_currentPoint.y - 800;
//    viewTop = m_currentPoint.y + 800;
    viewLeft = m_currentPoint.x - m_carLeftFieldOfVision;
    viewRight = m_currentPoint.x + m_carRightFieldOfVision;
    viewTop = m_currentPoint.y + m_carFrontFieldOfVision;
    viewBottom = m_currentPoint.y - m_carBackFieldOfVision;

    // 计算显示区域
    GLdouble sizeX = viewRight - viewLeft;
    GLdouble sizeY = sizeX * ((GLdouble)height() / (GLdouble)width());
    GLdouble scaleX = sizeX * m_fScaling;
    GLdouble scaleY = sizeY * m_fScaling;
    m_viewWindow.left = ((viewLeft + viewRight - scaleX) / 2.0 + m_moveDx);
    m_viewWindow.bottom = ((viewBottom + viewTop - scaleY) / 2.0 + m_moveDy);
    m_viewWindow.right = ((viewLeft + viewRight + scaleX) / 2.0 + m_moveDx);
    m_viewWindow.top = ((viewBottom + viewTop + scaleY) / 2.0 + m_moveDy);
    // 计算地图显示比例尺
    m_fMapScale = (m_viewWindow.right - m_viewWindow.left) / width();

//    COM_DEBUG(5, ("ADC: DrivingMapWidget::updateViewWindow, [(%f, %f) (%f, %f)]",
//                  m_viewWindow.left, m_viewWindow.bottom, m_viewWindow.right, m_viewWindow.top));

#endif
}

bool DrivingMapWidget::isClipedScreen(MAP_RECTANGLE& rect)
{
//    qDebug("window[(%f, %f, %f, %f)] rect[(%f, %f, %f, %f)]",
//           m_viewWindow.left, m_viewWindow.bottom, m_viewWindow.right, m_viewWindow.top,
//           rect.left, rect.bottom, rect.right, rect.top);
    if( (m_viewWindow.left - 400 > rect.right) ||
        (m_viewWindow.right + 400 < rect.left) ||
        (m_viewWindow.top + 200 < rect.bottom) ||
        (m_viewWindow.bottom - 200 > rect.top)) {
        // 图形不在视口内
        return true;
    }

    // 图形在视口内
    return false;
}

void DrivingMapWidget::recoverDefaultViewWindow()
{
    m_fScaling = 1.0;

    m_moveDx = 0.0;
    m_moveDy = 0.0;
}

void DrivingMapWidget::paintMap()
{
    if ( (!(m_pNaviData->m_mapData.isMapValid())) &&
         (!(m_pNaviData->m_historyTrace.isValid())) ) {
        return;
    }

    drawMap();
    drawHistoryTrace();
}

void DrivingMapWidget::paintCurrentTrace()
{
    drawCurrentTrace();
}

void DrivingMapWidget::paintCar()
{
    drawCar();
}

void DrivingMapWidget::paintBgImg()
{
    drawBgImg();
}

void DrivingMapWidget::paintObstacles()
{
    drawObstacles();
}

void DrivingMapWidget::paintDrivingInfo()
{
    drawDrivingInfo();
}

void DrivingMapWidget::paintCamInfo()
{
    drawCamInfo();
}

void DrivingMapWidget::paintMovementCtlInfo()
{
    drawMovementCtlInfo();
}

void DrivingMapWidget::drawMap()
{
    draw3DHighAccuracyData();
}

void DrivingMapWidget::draw3DHighAccuracyData()
{
    // 绘制地面标志
    drawPaintMark();
    // 绘制道路部(背景)
    drawRoadAreaHAD();
    // 绘制道路线
    drawRoadLaneLineHAD();

    //KHAData* pHAData = m_pHAData;
    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    if (NULL != pHAData) {
        // 遍历道路向量(KHI_ROAD_VECTOR)，查找需要显示的地图图元
        KHI_ROAD_VECTOR_LIST::const_iterator its_roadVector = pHAData->RoadVectors().begin();
        KHI_ROAD_VECTOR_LIST::const_iterator ite_roadVector = pHAData->RoadVectors().end();
        for (; its_roadVector != ite_roadVector; ++its_roadVector) {
            const KHI_ROAD_VECTOR* pRoadVector = (*its_roadVector);

//            kn_uint iRoadVectorIndex = 0;
//            kn_uint iRoadVectorShapeSize = 4;
            convertPointUnit(pRoadVector->key2.dwBoxLeft,
                             pRoadVector->key2.dwBoxBottom, m_tempPointBuff[0]);
            convertPointUnit(pRoadVector->key2.dwBoxRight,
                             pRoadVector->key2.dwBoxBottom, m_tempPointBuff[1]);
            convertPointUnit(pRoadVector->key2.dwBoxRight,
                             pRoadVector->key2.dwBoxTop, m_tempPointBuff[2]);
            convertPointUnit(pRoadVector->key2.dwBoxLeft,
                             pRoadVector->key2.dwBoxTop, m_tempPointBuff[3]);
            MAP_RECTANGLE box;
            box.left = m_tempPointBuff[0].x;
            box.right = m_tempPointBuff[1].x;
            box.top = m_tempPointBuff[2].y;
            box.bottom = m_tempPointBuff[1].y;

            if (isClipedScreen(box)) {
                continue;
            }

            // 绘制车道向量包围盒
            //qglColor(QColor(255, 0, 0));
            //glBegin(GL_LINE_LOOP);
            //for (iRoadVectorIndex = 0; iRoadVectorIndex < iRoadVectorShapeSize; iRoadVectorIndex++) {
            //    glVertex2d(m_tempPointBuff[iRoadVectorIndex].x - m_viewWindow.left, m_tempPointBuff[iRoadVectorIndex].y - m_viewWindow.bottom);
            //}
            //glEnd();

            // 绘制道路部(背景)

            // 绘制道路区间(KHI_LANE_SECTION)
            for (int SEQ = 1; SEQ <= pRoadVector->dwLaneSectionCount; ++SEQ) {
                const KHI_LANE_SECTION* pLaneSection = pHAData->LaneSection(KHI_LANE_SECTION::Key2(pRoadVector->key1.dwID, SEQ));
                drawLaneSection(pLaneSection);
            }

            // 绘制交叉路口(KHI_CROSS)
            const KHI_ROAD_CROSS* pRoadCross = pHAData->LaneCross(KHI_ROAD_CROSS::Key(pRoadVector->dwECrossID));
            drawRoadCross(pRoadCross);

//            // 绘制道路线(KHI_LANE_LINE)
//            if (NULL != pRoadVector->pLaneLines) {
//                for (int laneIndex = 0; laneIndex < pRoadVector->dwLaneLineCount; ++laneIndex) {
//                    const KHI_LANE_LINE* roadLaneLine = pHAData->LaneLine(KHI_LANE_LINE::Key(pRoadVector->pLaneLines[laneIndex]));
//                    if (NULL != roadLaneLine) {
//                        kn_uint iRoadLaneLineIndex = 0;
//                        kn_uint iRoadLaneLineShapeSize = roadLaneLine->dwPolylineShapeCount;

//                        if (iRoadLaneLineShapeSize > MAX_POINT_BUFF) {
//                            qDebug("MapWidget::drawRoadLaneLineHAD, iShapeSize(%d) too long", iRoadLaneLineShapeSize);
//                            QMessageBox::warning(this,
//                                                 tr("Autonomous Driving Control"),
//                                                 tr("Draw Road Lane Line HAD, shape size too long"),
//                                                 QMessageBox::Yes);

//                            iRoadLaneLineShapeSize = MAX_POINT_BUFF;
//                        }
//                        MAP_RECTANGLE box;
//                        box.left = 999999999.0;
//                        box.right = -999999999.0;
//                        box.top = -999999999.0;
//                        box.bottom = 999999999.0;
//                        for (iRoadLaneLineIndex = 0; iRoadLaneLineIndex < iRoadLaneLineShapeSize; iRoadLaneLineIndex++) {
//                            convertPointUnit(roadLaneLine->pPolylineShapes[iRoadLaneLineIndex].x,
//                                             roadLaneLine->pPolylineShapes[iRoadLaneLineIndex].y,
//                                             m_tempPointBuff[iRoadLaneLineIndex]);

//                            box.left = box.left > m_tempPointBuff[iRoadLaneLineIndex].x ?
//                                        m_tempPointBuff[iRoadLaneLineIndex].x : box.left;
//                            box.right = box.right < m_tempPointBuff[iRoadLaneLineIndex].x ?
//                                        m_tempPointBuff[iRoadLaneLineIndex].x : box.right;
//                            box.top = box.top < m_tempPointBuff[iRoadLaneLineIndex].y ?
//                                        m_tempPointBuff[iRoadLaneLineIndex].y : box.top;
//                            box.bottom = box.bottom > m_tempPointBuff[iRoadLaneLineIndex].y ?
//                                        m_tempPointBuff[iRoadLaneLineIndex].y : box.bottom;
//                        }

//                        if (isClipedScreen(box)) {
//                            continue;
//                        }

//                        qglColor(QColor(255, 255, 255));

//                        if (roadLaneLine->dwColor == 0) {
//                            qglColor(QColor(255, 255, 255));
//                        }
//                        else if (roadLaneLine->dwColor == 1) {
//                            qglColor(QColor(255, 255, 0));
//                        }

//                        switch (roadLaneLine->dwStyle) {
//                        case LINESTYLE_SINGLE_DOT: // 虚线
//                            //
//                            break;

//                        case LINESTYLE_SINGLE_FULL: // 实线
//                            if (roadLaneLine->dwColor == 1) {
//                                //
//                            }
//                            else {
//                                //
//                            }
//                            break;

//                        case LINESTYLE_DOUBLE_FULL: // 双实线
//                            //
//                            break;

//                        case LINESTYLE_FULL_DOT: // 双线（左实右虚）
//                            //
//                            break;

//                        case LINESTYLE_DOT_FULL: // 双线（左虚右实）
//                            //
//                            break;

//                        default:
//                            break;
//                        }

//                        glBegin(GL_LINE_STRIP);
//                        for (iRoadLaneLineIndex = 0; iRoadLaneLineIndex < iRoadLaneLineShapeSize; iRoadLaneLineIndex++) {
//                            glVertex2d(m_tempPointBuff[iRoadLaneLineIndex].x - m_viewWindow.left,
//                                       m_tempPointBuff[iRoadLaneLineIndex].y - m_viewWindow.bottom);
//                        }
//                        glEnd();
//                    }
//                }
//            }
        }
    }
}

void DrivingMapWidget::drawLaneSection(const KHI_LANE_SECTION* pSection)
{
    if (NULL == pSection) {
        return;
    }

    double tmpDistance = 0.0;
    MAP_POINT tmpPoint;
    kn_dword dwLaneVecID         = pSection->dwRoadVecID;
    kn_dword dwSEQ               = pSection->dwSEQ;
    kn_dword dwCentralLaneCount  = pSection->dwCentralLaneCount;

    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    // 绘制车道中心线
    qglColor(QColor(250, 200, 230));
    //qglColor(QColor(255, 128, 64));
    glLineStipple(3, 0x3F07);
    glEnable(GL_LINE_STIPPLE);

    for (int LaneNum = 1; LaneNum <= dwCentralLaneCount; ++LaneNum) {
        const KHI_LANE_CENTRAL* pCentral = pHAData->LaneCentral(KHI_LANE_CENTRAL::Key(dwLaneVecID, dwSEQ, LaneNum));
        if (NULL != pCentral) {
            // 找到对应的车道中心线(KHI_LANE_Central)
            if (const KHI_LINE* pLine = pHAData->Line(KHI_LINE::Key(pCentral->dwLineID))) {
                kn_uint ii = 0;
                kn_uint iShapeSize = pLine->dwPtCount;

                if (iShapeSize > MAX_POINT_BUFF) {
                    qDebug("MapWidget::drawLaneCentral, iShapeSize(%d) too long", iShapeSize);
                    QMessageBox::warning(this,
                                         tr("Autonomous Driving Control"),
                                         tr("Draw lane central, shape size too long"),
                                         QMessageBox::Yes);

                    iShapeSize = MAX_POINT_BUFF;
                }
                MAP_RECTANGLE box;
                box.left = 999999999.0;
                box.right = -999999999.0;
                box.top = -999999999.0;
                box.bottom = 999999999.0;
                for (ii = 0; ii < iShapeSize; ii++) {
                    convertPointUnit(pLine->pPoints[ii].x, pLine->pPoints[ii].y, m_tempPointBuff[ii]);

                    box.left = box.left > m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.left;
                    box.right = box.right < m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.right;
                    box.top = box.top < m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.top;
                    box.bottom = box.bottom > m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.bottom;
                }

                if (isClipedScreen(box)) {
                    continue;
                }

                glBegin(GL_LINE_STRIP);
                for (ii = 0; ii < iShapeSize; ii++)
                {
                    glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
                    // 计算最近的道路中心线
                    if (ii > 0) {
                        tmpDistance = getPointToSegDist(m_currentPoint,
                                                    m_tempPointBuff[ii - 1],
                                                    m_tempPointBuff[ii],
                                                    tmpPoint);
                    } else {
                        tmpDistance = sqrt((m_currentPoint.x - m_tempPointBuff[ii].x) * (m_currentPoint.x - m_tempPointBuff[ii].x) +
                                           (m_currentPoint.y - m_tempPointBuff[ii].y) * (m_currentPoint.y - m_tempPointBuff[ii].y));
                        tmpPoint = m_tempPointBuff[ii];
                    }
                    if (tmpDistance < m_drivingInfo.nearestDistance) {
                        m_drivingInfo.nearestDistance = tmpDistance;
                        m_drivingInfo.nearestPoint = tmpPoint;
                    }
                }
                glEnd();

                glPointSize(4);
                glBegin(GL_POINTS);
                for (ii = 0; ii < iShapeSize; ii++)
                {
                    glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
                }
                glEnd();
            }
        }
    }

    glDisable(GL_LINE_STIPPLE);
}

void DrivingMapWidget::drawRoadCross(const KHI_ROAD_CROSS* pCross)
{
    if (NULL == pCross) {
        return;
    }

    kn_dword dwCrossID = pCross->key.dwId;
    kn_dword dwDummyLaneCount = pCross->dwDummyLaneCount;

    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    // 绘制dummy连接线(KHI_DUMMY_LINE)
    qglColor(QColor(170, 180, 240));
    //qglColor(QColor(255, 128, 0));

    glLineStipple(3, 0x3F07);
    glEnable(GL_LINE_STIPPLE);

    for (int SEQ = 0; SEQ < dwDummyLaneCount; ++SEQ) {
        const KHI_DUMMY_LINE* pDummy = pHAData->DummyLine(KHI_DUMMY_LINE::Key(dwCrossID, SEQ));

        if (NULL != pDummy) {
            kn_uint ii = 0;
            kn_uint iShapeSize = pDummy->dwPtCount;

            if (iShapeSize > MAX_POINT_BUFF) {
                qDebug("MapWidget::drawDummyLine, iShapeSize(%d) too long", iShapeSize);
                QMessageBox::warning(this,
                                     tr("Autonomous Driving Control"),
                                     tr("Draw dummy line, shape size too long"),
                                     QMessageBox::Yes);

                iShapeSize = MAX_POINT_BUFF;
            }
            MAP_RECTANGLE box;
            box.left = 999999999.0;
            box.right = -999999999.0;
            box.top = -999999999.0;
            box.bottom = 999999999.0;
            for (ii = 0; ii < iShapeSize; ii++) {
                convertPointUnit(pDummy->pPoints[ii].x, pDummy->pPoints[ii].y, m_tempPointBuff[ii]);

                box.left = box.left > m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.left;
                box.right = box.right < m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.right;
                box.top = box.top < m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.top;
                box.bottom = box.bottom > m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.bottom;
            }

            if (isClipedScreen(box)) {
                continue;
            }

            glBegin(GL_LINE_STRIP);
            //glBegin(GL_POINTS);
            for (ii = 0; ii < iShapeSize; ii++) {
                glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
            }
            glEnd();
        }
    }

    glDisable(GL_LINE_STIPPLE);
}

void DrivingMapWidget::drawRoadLaneLineHAD()
{
    //qDebug("DrivingMapWidget::drawRoadLaneLineHAD");

    //KHAData* pHAData = m_pMapData->m_pHAData;
    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    if (NULL == pHAData) {
        return;
    }

    KHI_LANE_LINE_LIST::const_iterator its = pHAData->LaneLines().begin();
    KHI_LANE_LINE_LIST::const_iterator ite = pHAData->LaneLines().end();
    for ( ; its != ite; ++its)
    {
        const KHI_LANE_LINE* roadLaneLine = (*its);

        kn_uint ii = 0;
        kn_uint iShapeSize = roadLaneLine->dwPolylineShapeCount;

        if (iShapeSize > MAX_POINT_BUFF) {
            qDebug("DrivingMapWidget::drawRoadLaneLineHAD, iShapeSize(%d) too long", iShapeSize);
            QMessageBox::warning(this,
                                 tr("Autonomous Driving Control"),
                                 tr("Draw Road Lane Line HAD, shape size too long"),
                                 QMessageBox::Yes);

            iShapeSize = MAX_POINT_BUFF;
        }
        MAP_RECTANGLE box;
        box.left = 999999999.0;
        box.right = -999999999.0;
        box.top = -999999999.0;
        box.bottom = 999999999.0;
        for (ii = 0; ii < iShapeSize; ii++) {
            convertPointUnit(roadLaneLine->pPolylineShapes[ii].x, roadLaneLine->pPolylineShapes[ii].y, m_tempPointBuff[ii]);

            box.left = box.left > m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.left;
            box.right = box.right < m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.right;
            box.top = box.top < m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.top;
            box.bottom = box.bottom > m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.bottom;
        }

        if (isClipedScreen(box)) {
            continue;
        }

        qglColor(QColor(255, 255, 255));

        if (roadLaneLine->dwColor == 0) {
            qglColor(QColor(255, 255, 255));
        }
        else if (roadLaneLine->dwColor == 1) {
            qglColor(QColor(255, 255, 0));
        }

        switch (roadLaneLine->dwStyle) {
        case LINESTYLE_SINGLE_DOT: // 虚线
            //
            break;

        case LINESTYLE_SINGLE_FULL: // 实线
            if (roadLaneLine->dwColor == 1) {
                //
            }
            else {
                //
            }
            break;

        case LINESTYLE_DOUBLE_FULL: // 双实线
            //
            break;

        case LINESTYLE_FULL_DOT: // 双线（左实右虚）
            //
            break;

        case LINESTYLE_DOT_FULL: // 双线（左虚右实）
            //
            break;

        default:
            break;
        }

        glBegin(GL_LINE_STRIP);
        for (ii = 0; ii < iShapeSize; ii++) {
            glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
        }
        glEnd();
    }
}

void DrivingMapWidget::drawRoadAreaHAD()
{
    //qDebug("DrivingMapWidget::drawRoadAreaHAD");

    //KHAData* pHAData = m_pMapData->m_pHAData;
    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    if (NULL == pHAData) {
        return;
    }

    qglColor(QColor(114, 123, 133));

    KHI_ROAD_AREA_LIST::const_iterator its = pHAData->RoadAreas().begin();
    KHI_ROAD_AREA_LIST::const_iterator ite = pHAData->RoadAreas().end();

    for ( ; its != ite; ++its) {
        const KHI_ROAD_AREA* roadArea = (*its);

        kn_uint ii = 0;
        kn_uint iShapeSize = roadArea->dwPolygonShapeCount;

        if (iShapeSize > MAX_POINT_BUFF) {
            qDebug("DrivingMapWidget::drawRoadAreaHAD, iShapeSize(%d) too long", iShapeSize);
            QMessageBox::warning(this,
                                 tr("Autonomous Driving Control"),
                                 tr("Draw Road Area HAD, shape size too long"),
                                 QMessageBox::Yes);

            iShapeSize = MAX_POINT_BUFF;
        }
        if (0 == iShapeSize) {
            continue;
        }
        MAP_RECTANGLE box;
        box.left = 999999999.0;
        box.right = -999999999.0;
        box.top = -999999999.0;
        box.bottom = 999999999.0;
        for (ii = 0; ii < iShapeSize; ii++) {
            convertPointUnit(roadArea->pPolygonShapes[ii].x, roadArea->pPolygonShapes[ii].y, m_tempPointBuff[ii]);

            box.left = box.left > m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.left;
            box.right = box.right < m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.right;
            box.top = box.top < m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.top;
            box.bottom = box.bottom > m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.bottom;
        }

        if (isClipedScreen(box)) {
            continue;
        }

        glBegin(GL_POLYGON);
        for (ii = 0; ii < iShapeSize; ii++) {
            glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
            //m_ptShape[ii] = m_ViewportManager.LogicalToDevice(roadArea->pPolygonShapes[ii].x, roadArea->pPolygonShapes[ii].y);
        }
        glEnd();
    }
}

void DrivingMapWidget::drawPaintMark()
{
    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    if (NULL == pHAData) {
        return;
    }

    KHI_PAINT_MARK_LIST::const_iterator its = pHAData->PaintMarks().begin();
    KHI_PAINT_MARK_LIST::const_iterator ite = pHAData->PaintMarks().end();
    for ( ; its != ite; ++its) {
        const KHI_PAINT_MARK* paintMark = (*its);

        bool bIsDrawBackground = false;
        bool bIsDrawIsolation = false;
        bool bIsSteeringAngle = false;
        bool bIsGreenBelt = false;

        if (paintMark->dwType == PMT_PEDESTRIAN_CROSSING || paintMark->dwType == PMT_6) {
            // 人行横道、安全带
            bIsDrawBackground = true;
            bIsDrawIsolation = true;
        }
        else if (paintMark->dwType == PMT_DIRECTION) {
            //bIsDrawBackground = true;
            bIsSteeringAngle = true;
        }
        else if (paintMark->dwType == PMT_11) {
            // 绿化带
            bIsDrawBackground = true;
        }

        // 获取箭头方向的矢量
        KNGEOCOORD ArrowPt1;
        KNGEOCOORD ArrowPt2;
        KNGEOCOORD ArrowCentrePt;
        kn_float fArrowAngle1 = 0.0f;       // 矢量方向
        kn_float fArrowAngle2 = 0.0f;       // 反向 180度
        kn_float fArrowAngle3 = 0.0f;       // 左转 90度
        kn_float fArrowAngle4 = 0.0f;       // 右转 90度
        if (const KHI_LINE* pArrow = pHAData->Line(KHI_LINE::Key(paintMark->dwArrowLineID))) {
            kn_uint iShapeSize = pArrow->dwPtCount;
            if (iShapeSize == 2) {
                ArrowPt1.ulLongitude    = pArrow->pPoints[0].x;
                ArrowPt1.ulLatitude     = pArrow->pPoints[0].y;
                ArrowPt1.fLongitude     = pArrow->pPoints[0].x;
                ArrowPt1.fLatitude      = pArrow->pPoints[0].y;

                ArrowPt2.ulLongitude    = pArrow->pPoints[1].x;
                ArrowPt2.ulLatitude     = pArrow->pPoints[1].y;
                ArrowPt2.fLongitude     = pArrow->pPoints[1].x;
                ArrowPt2.fLatitude      = pArrow->pPoints[1].y;

                ArrowCentrePt.fLongitude    = (ArrowPt1.ulLongitude + ArrowPt2.ulLongitude) * 0.5f;
                ArrowCentrePt.fLatitude     = (ArrowPt1.ulLatitude + ArrowPt2.ulLatitude) * 0.5f;
                ArrowCentrePt.ulLongitude   = ArrowCentrePt.fLongitude;
                ArrowCentrePt.ulLatitude    = ArrowCentrePt.fLatitude;

                fArrowAngle1 = CalAngle(ArrowPt1, ArrowPt2);
                fArrowAngle2 = fArrowAngle1 + 180.0f;
                while (fArrowAngle2 > 360.0f) {
                    fArrowAngle2 -= 360.0f;
                }

                fArrowAngle3 = fArrowAngle1 + 90.0f;
                while (fArrowAngle3 > 360.0f) {
                    fArrowAngle3 -= 360.0f;
                }
                fArrowAngle4 = fArrowAngle1 - 90.0f;
                while (fArrowAngle4 < 0.0f) {
                    fArrowAngle4 += 360.0f;
                }
            } else {
                continue;
            }
        }

        // 避免屏幕外的绘制
        if (!bIsDrawBackground) {
            MAP_RECTANGLE box;
            box.left = 999999999.0;
            box.right = -999999999.0;
            box.top = -999999999.0;
            box.bottom = 999999999.0;

            convertPointUnit(ArrowPt1.fLongitude, ArrowPt1.fLatitude, m_tempPointBuff[0]);
            box.left = box.left > m_tempPointBuff[0].x ? m_tempPointBuff[0].x : box.left;
            box.right = box.right < m_tempPointBuff[0].x ? m_tempPointBuff[0].x : box.right;
            box.top = box.top < m_tempPointBuff[0].y ? m_tempPointBuff[0].y : box.top;
            box.bottom = box.bottom > m_tempPointBuff[0].y ? m_tempPointBuff[0].y : box.bottom;

            convertPointUnit(ArrowPt2.fLongitude, ArrowPt2.fLatitude, m_tempPointBuff[1]);
            box.left = box.left > m_tempPointBuff[1].x ? m_tempPointBuff[1].x : box.left;
            box.right = box.right < m_tempPointBuff[1].x ? m_tempPointBuff[1].x : box.right;
            box.top = box.top < m_tempPointBuff[1].y ? m_tempPointBuff[1].y : box.top;
            box.bottom = box.bottom > m_tempPointBuff[1].y ? m_tempPointBuff[1].y : box.bottom;

            if (isClipedScreen(box)) {
                continue;
            }
        }

        if (const KHI_FACE* pFace = pHAData->Face(KHI_FACE::Key(paintMark->dwShapeID))) {
            kn_uint ii = 0;
            kn_uint iShapeSize = pFace->dwPtCount;

            MAP_RECTANGLE rctScope;
            rctScope.left   = 999999999.0;
            rctScope.bottom = 999999999.0;
            rctScope.right  = -999999999.0;
            rctScope.top    = -999999999.0;

            MAP_RECTANGLE box;
            box.left = 999999999.0;
            box.right = -999999999.0;
            box.top = -999999999.0;
            box.bottom = 999999999.0;
            for (ii = 0; ii < iShapeSize; ii++) {
                // 查找包围盒的外接矩形
                if (pFace->pPoints[ii].x < rctScope.left) {
                    rctScope.left = pFace->pPoints[ii].x;
                }
                if (pFace->pPoints[ii].x > rctScope.right) {
                    rctScope.right = pFace->pPoints[ii].x;
                }
                if (pFace->pPoints[ii].y < rctScope.bottom) {
                    rctScope.bottom = pFace->pPoints[ii].y;
                }
                if (pFace->pPoints[ii].y > rctScope.top) {
                    rctScope.top = pFace->pPoints[ii].y;
                }

                convertPointUnit(pFace->pPoints[ii].x,
                                 pFace->pPoints[ii].y, m_tempPointBuff[ii]);

                box.left = box.left > m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.left;
                box.right = box.right < m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.right;
                box.top = box.top < m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.top;
                box.bottom = box.bottom > m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.bottom;
            }

            if (isClipedScreen(box)) {
                continue;
            }

            // 绘制背景
            if (bIsDrawBackground) {
                if (paintMark->dwType == PMT_PEDESTRIAN_CROSSING || paintMark->dwType == PMT_6) {
                    // 人行横道、安全带
                    qglColor(QColor(110, 110, 110));
                    glBegin(GL_POLYGON);
                    for (int ii = 0; ii < iShapeSize; ii++) {
                        glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
                    }
                    glEnd();
                }
                else if (paintMark->dwType == PMT_11) {
                    // 绿化带
                    qglColor(QColor(186, 216, 178));
                    glBegin(GL_POLYGON);
                    for (int ii = 0; ii < iShapeSize; ii++) {
                        glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
                    }
                    glEnd();
                    continue;
                }
            }

#if 1
            // 绘制 隔线
            if (bIsDrawIsolation) {
                // 1. 查找外接矩形最长边
                KNGEOCOORD point1(rctScope.left, rctScope.top);
                KNGEOCOORD point2(rctScope.right, rctScope.top);
                KNGEOCOORD point3(rctScope.right, rctScope.bottom);
                kn_double fL1 = CalcSphericalDistanceF(point1, point2);
                kn_double fL2 = CalcSphericalDistanceF(point2, point3);
                kn_double fMaxL = (fL1 > fL2)? fL1 : fL2;
                // 2. 向外扩展箭头的二个端点
                KNGEOCOORD ArrowPtEx1 = GetNextPoint(ArrowPt1, fArrowAngle2, fMaxL);
                KNGEOCOORD ArrowPtEx2 = GetNextPoint(ArrowPt2, fArrowAngle1, fMaxL);
                // 3. 计算分隔线段
                kn_float fIntervalDis = 0.7f;

                // 向左扫描
                KPoint p1 = ArrowPtEx1;
                KPoint p2 = ArrowPtEx2;
                kn_bool bIsScan = true;
                while (bIsScan) {
                    kn_int iPointIndex = 0;
                    KPoint asnPont1;
                    KPoint asnPont2;
                    for (ii = 0; ii < iShapeSize-1; ii++) {
                        KPoint p3(pFace->pPoints[ii].x, pFace->pPoints[ii].y);
                        KPoint p4(pFace->pPoints[ii+1].x, pFace->pPoints[ii+1].y);
                        if (iPointIndex == 0) {
                            bool bPoint = GetIntersectPoint(p1, p2, p3, p4, asnPont1);
                            if (bPoint) {
                                iPointIndex += 1;
                                continue;
                            }
                        } else if (iPointIndex == 1) {
                            bool bPoint = GetIntersectPoint(p1, p2, p3, p4, asnPont2);
                            if (bPoint) {
                                iPointIndex += 1;
                                break;
                            }
                        }
                    }

                    if (iPointIndex == 2) {
                        // 绘制线段
                        //vctPos1 = m_ViewportManager.LogicalToDevice(asnPont1.X, asnPont1.Y);
                        //vctPos2 = m_ViewportManager.LogicalToDevice(asnPont2.X, asnPont2.Y);
                        //m_pSurfMain->LineEx(vctPos1.x, vctPos1.y, vctPos2.x, vctPos2.y, 5, lineColor, 1);
                        convertPointUnit(asnPont1.X, asnPont1.Y, m_tempPointBuff[0]);
                        convertPointUnit(asnPont2.X, asnPont2.Y, m_tempPointBuff[1]);
                        qglColor(QColor(255, 255, 255));
                        glLineWidth(2);
                        glBegin(GL_LINES);
                        for (int i = 0; i < iShapeSize; i++) {
                            glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                        }
                        glEnd();
                        glLineWidth(1);

                        // 平移扫描线
                        KNGEOCOORD crdP1(p1.X, p1.Y);
                        crdP1.set((double)p1.X, (double)p1.Y);
                        KNGEOCOORD crdP2(p2.X, p2.Y);
                        crdP2.set((double)p2.X, (double)p2.Y);
                        crdP1 = GetNextPoint(crdP1, fArrowAngle3, fIntervalDis);
                        crdP2 = GetNextPoint(crdP2, fArrowAngle3, fIntervalDis);
                        p1 = crdP1;
                        p2 = crdP2;
                    }

                    if (iPointIndex < 2) {
                        bIsScan = false;
                    }

                    //MyTrace(_T("DrawPaintMark ---------- left iPointIndex: %d        \n"), iPointIndex);
                }

                // 向右扫描
                p1 = ArrowPtEx1;
                p2 = ArrowPtEx2;
                bIsScan = true;
                while (bIsScan) {
                    kn_int iPointIndex = 0;
                    KPoint asnPont1;
                    KPoint asnPont2;
                    for (ii = 0; ii < iShapeSize-1; ii++) {
                        KPoint p3(pFace->pPoints[ii].x, pFace->pPoints[ii].y);
                        KPoint p4(pFace->pPoints[ii+1].x, pFace->pPoints[ii+1].y);
                        if (iPointIndex == 0) {
                            bool bPoint = GetIntersectPoint(p1, p2, p3, p4, asnPont1);
                            if (bPoint) {
                                iPointIndex += 1;
                                continue;
                            }
                        } else if (iPointIndex == 1) {
                            bool bPoint = GetIntersectPoint(p1, p2, p3, p4, asnPont2);
                            if (bPoint) {
                                iPointIndex += 1;
                                break;
                            }
                        }
                    }

                    if (iPointIndex == 2) {
                        // 绘制线段
                        //vctPos1 = m_ViewportManager.LogicalToDevice(asnPont1.X, asnPont1.Y);
                        //vctPos2 = m_ViewportManager.LogicalToDevice(asnPont2.X, asnPont2.Y);
                        //m_pSurfMain->LineEx(vctPos1.x, vctPos1.y, vctPos2.x, vctPos2.y, 5, lineColor, 1);
                        convertPointUnit(asnPont1.X, asnPont1.Y, m_tempPointBuff[0]);
                        convertPointUnit(asnPont2.X, asnPont2.Y, m_tempPointBuff[1]);
                        qglColor(QColor(255, 255, 255));
                        glLineWidth(2);
                        glBegin(GL_LINES);
                        for (int i = 0; i < iShapeSize; i++) {
                            glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                        }
                        glEnd();
                        glLineWidth(1);

                        // 平移扫描线
                        KNGEOCOORD crdP1(p1.X, p1.Y);
                        crdP1.set((double)p1.X, (double)p1.Y);
                        KNGEOCOORD crdP2(p2.X, p2.Y);
                        crdP2.set((double)p2.X, (double)p2.Y);
                        crdP1 = GetNextPoint(crdP1, fArrowAngle4, fIntervalDis);
                        crdP2 = GetNextPoint(crdP2, fArrowAngle4, fIntervalDis);
                        p1 = crdP1;
                        p2 = crdP2;
                    }

                    if (iPointIndex < 2) {
                        bIsScan = false;
                    }
                }
            }
#endif

#if 1
            // 绘制 转向指示
            if (bIsSteeringAngle) {
                //kn_word wContent = paintMark->dwTextID;
                kn_word wContent = paintMark->wDirectionType;

                qglColor(QColor(255, 255, 255));

                if ((wContent & 1) == 1) {
                    // 直行
                    // 1. 箭头
                    kn_float fTriangleL1 = 1.5f;
                    kn_float fTriangleL2 = 0.5f;
                    KNGEOCOORD crdTri1 = GetNextPoint(ArrowCentrePt, fArrowAngle1, fTriangleL1);
                    KNGEOCOORD crdTri2 = GetNextPoint(ArrowCentrePt, fArrowAngle3, fTriangleL2);
                    KNGEOCOORD crdTri3 = GetNextPoint(ArrowCentrePt, fArrowAngle4, fTriangleL2);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdTri1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdTri2);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdTri3);
                    //m_pSurfMain->Polygon(m_ptShape, 3, steeringColor);
                    convertPointUnit(crdTri1.fLongitude, crdTri1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdTri2.fLongitude, crdTri2.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdTri3.fLongitude, crdTri3.fLatitude, m_tempPointBuff[2]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 3; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();
                } else if ((wContent & 2) == 2 || (wContent & 8) == 8 ) {
                    // 左转  前左转
                    kn_float fFlagpoleL0 = 0.25f;
                    kn_float fFlagpoleL1 = 0.5f;
                    KNGEOCOORD crdFlag1 = GetNextPoint(ArrowCentrePt, fArrowAngle3, fFlagpoleL0);
                    KNGEOCOORD crdFlag2 = GetNextPoint(ArrowCentrePt, fArrowAngle4, fFlagpoleL0);
                    KNGEOCOORD crdFlag3 = GetNextPoint(crdFlag2, fArrowAngle1, fFlagpoleL1);
                    KNGEOCOORD crdFlag4 = GetNextPoint(crdFlag1, fArrowAngle1, fFlagpoleL1);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdFlag1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdFlag2);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdFlag3);
                    //m_ptShape[3] = m_ViewportManager.LogicalToDevice(crdFlag4);
                    //m_pSurfMain->Polygon(m_ptShape, 4, steeringColor);
                    convertPointUnit(crdFlag1.fLongitude, crdFlag1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdFlag2.fLongitude, crdFlag2.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdFlag3.fLongitude, crdFlag3.fLatitude, m_tempPointBuff[2]);
                    convertPointUnit(crdFlag4.fLongitude, crdFlag4.fLatitude, m_tempPointBuff[3]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 4; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();


                    kn_float fFlagpoleL2 = 0.2f;
                    kn_float fFlagpoleL3 = 0.05f;
                    KNGEOCOORD crdFlag5 = GetNextPoint(crdFlag1, fArrowAngle3, fFlagpoleL2);
                    KNGEOCOORD crdFlag6 = GetNextPoint(crdFlag4, fArrowAngle3, fFlagpoleL2);
                    KNGEOCOORD crdFlag7 = GetNextPoint(crdFlag5, fArrowAngle3, fFlagpoleL3);
                    KNGEOCOORD crdFlag8 = GetNextPoint(crdFlag6, fArrowAngle3, fFlagpoleL3);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdFlag1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdFlag4);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdFlag8);
                    //m_ptShape[3] = m_ViewportManager.LogicalToDevice(crdFlag7);
                    //m_pSurfMain->Polygon(m_ptShape, 4, steeringColor);
                    convertPointUnit(crdFlag1.fLongitude, crdFlag1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdFlag4.fLongitude, crdFlag4.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdFlag8.fLongitude, crdFlag8.fLatitude, m_tempPointBuff[2]);
                    convertPointUnit(crdFlag7.fLongitude, crdFlag7.fLatitude, m_tempPointBuff[3]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 4; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();

                    // 1. 箭头
                    kn_float fTriangleL1 = 0.5f;
                    kn_float fTriangleL2 = 0.7f;
                    KNGEOCOORD crdTri1((crdFlag5.ulLongitude + crdFlag6.ulLongitude) / 2, (crdFlag5.ulLatitude + crdFlag6.ulLatitude) / 2);
                    crdTri1.set((crdFlag5.fLongitude + crdFlag6.ulLongitude) / 2.0f, (crdFlag5.fLatitude + crdFlag6.fLatitude) / 2.0f);
                    crdTri1 = GetNextPoint(crdTri1, fArrowAngle3, fTriangleL1);
                    KNGEOCOORD crdTri2 = GetNextPoint(crdFlag6, fArrowAngle1, fTriangleL2);
                    KNGEOCOORD crdTri3 = GetNextPoint(crdFlag5, fArrowAngle2, fTriangleL2);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdTri1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdTri2);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdTri3);
                    //m_pSurfMain->Polygon(m_ptShape, 3, steeringColor);
                    convertPointUnit(crdTri1.fLongitude, crdTri1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdTri2.fLongitude, crdTri2.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdTri3.fLongitude, crdTri3.fLatitude, m_tempPointBuff[2]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 3; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();
                } else if ((wContent & 4) == 4 || (wContent & 16) == 16 ) {
                    // 右转   前右转
                    kn_float fFlagpoleL0 = 0.25f;
                    kn_float fFlagpoleL1 = 0.5f;
                    KNGEOCOORD crdFlag1 = GetNextPoint(ArrowCentrePt, fArrowAngle3, fFlagpoleL0);
                    KNGEOCOORD crdFlag2 = GetNextPoint(ArrowCentrePt, fArrowAngle4, fFlagpoleL0);
                    KNGEOCOORD crdFlag3 = GetNextPoint(crdFlag2, fArrowAngle1, fFlagpoleL1);
                    KNGEOCOORD crdFlag4 = GetNextPoint(crdFlag1, fArrowAngle1, fFlagpoleL1);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdFlag1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdFlag2);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdFlag3);
                    //m_ptShape[3] = m_ViewportManager.LogicalToDevice(crdFlag4);
                    //m_pSurfMain->Polygon(m_ptShape, 4, steeringColor);
                    convertPointUnit(crdFlag1.fLongitude, crdFlag1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdFlag2.fLongitude, crdFlag2.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdFlag3.fLongitude, crdFlag3.fLatitude, m_tempPointBuff[2]);
                    convertPointUnit(crdFlag4.fLongitude, crdFlag4.fLatitude, m_tempPointBuff[3]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 4; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();

                    kn_float fFlagpoleL2 = 0.2f;
                    kn_float fFlagpoleL3 = 0.05f;
                    KNGEOCOORD crdFlag5 = GetNextPoint(crdFlag2, fArrowAngle4, fFlagpoleL2);
                    KNGEOCOORD crdFlag6 = GetNextPoint(crdFlag3, fArrowAngle4, fFlagpoleL2);
                    KNGEOCOORD crdFlag7 = GetNextPoint(crdFlag5, fArrowAngle4, fFlagpoleL3);
                    KNGEOCOORD crdFlag8 = GetNextPoint(crdFlag6, fArrowAngle4, fFlagpoleL3);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdFlag2);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdFlag3);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdFlag8);
                    //m_ptShape[3] = m_ViewportManager.LogicalToDevice(crdFlag7);
                    //m_pSurfMain->Polygon(m_ptShape, 4, steeringColor);
                    convertPointUnit(crdFlag2.fLongitude, crdFlag2.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdFlag3.fLongitude, crdFlag3.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdFlag8.fLongitude, crdFlag8.fLatitude, m_tempPointBuff[2]);
                    convertPointUnit(crdFlag7.fLongitude, crdFlag7.fLatitude, m_tempPointBuff[3]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 4; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();

                    // 1. 箭头
                    kn_float fTriangleL1 = 0.5f;
                    kn_float fTriangleL2 = 0.7f;
                    KNGEOCOORD crdTri1((crdFlag5.ulLongitude + crdFlag6.ulLongitude) / 2, (crdFlag5.ulLatitude + crdFlag6.ulLatitude) / 2);
                    crdTri1.set((crdFlag5.fLongitude + crdFlag6.fLongitude) / 2.0f, (crdFlag5.fLatitude + crdFlag6.fLatitude) / 2.0f);
                    crdTri1 = GetNextPoint(crdTri1, fArrowAngle4, fTriangleL1);
                    KNGEOCOORD crdTri2 = GetNextPoint(crdFlag6, fArrowAngle1, fTriangleL2);
                    KNGEOCOORD crdTri3 = GetNextPoint(crdFlag5, fArrowAngle2, fTriangleL2);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdTri1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdTri2);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdTri3);
                    //m_pSurfMain->Polygon(m_ptShape, 3, steeringColor);
                    convertPointUnit(crdTri1.fLongitude, crdTri1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdTri2.fLongitude, crdTri2.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdTri3.fLongitude, crdTri3.fLatitude, m_tempPointBuff[2]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 3; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();
                } else if ((wContent & 32) == 32 || (wContent & 128) == 128 ) {
                    // 左侧掉头 , 前左转 + 左侧掉头
                    kn_float fFlagpoleL0 = 0.25f;
                    kn_float fFlagpoleL1 = 0.5f;
                    kn_float fFlagpoleL2 = fFlagpoleL1 * 1.5f;
                    kn_float fFlagpoleL3 = fFlagpoleL1;
                    KNGEOCOORD crdFlag1 = GetNextPoint(ArrowCentrePt, fArrowAngle3, fFlagpoleL0);
                    KNGEOCOORD crdFlag2 = GetNextPoint(ArrowCentrePt, fArrowAngle4, fFlagpoleL0);
                    KNGEOCOORD crdFlag3 = GetNextPoint(crdFlag2, fArrowAngle1, fFlagpoleL1);
                    KNGEOCOORD crdFlag4 = GetNextPoint(crdFlag1, fArrowAngle1, fFlagpoleL1);
                    KNGEOCOORD crdFlag5 = GetNextPoint(crdFlag2, fArrowAngle3, fFlagpoleL2);
                    KNGEOCOORD crdFlag6 = GetNextPoint(crdFlag3, fArrowAngle3, fFlagpoleL2);
                    KNGEOCOORD crdFlag7 = GetNextPoint(crdFlag5, fArrowAngle3, fFlagpoleL3);
                    KNGEOCOORD crdFlag8 = GetNextPoint(crdFlag6, fArrowAngle3, fFlagpoleL3);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdFlag2);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdFlag3);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdFlag8);
                    //m_ptShape[3] = m_ViewportManager.LogicalToDevice(crdFlag7);
                    //m_pSurfMain->Polygon(m_ptShape, 4, steeringColor);
                    convertPointUnit(crdFlag2.fLongitude, crdFlag2.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdFlag3.fLongitude, crdFlag3.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdFlag8.fLongitude, crdFlag8.fLatitude, m_tempPointBuff[2]);
                    convertPointUnit(crdFlag7.fLongitude, crdFlag7.fLatitude, m_tempPointBuff[3]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 4; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();

                    kn_float fFlagpoleL4 = 0.2f;
                    kn_float fFlagpoleL5 = 0.05f;
                    KNGEOCOORD crdFlag9 = GetNextPoint(crdFlag5, fArrowAngle2, fFlagpoleL4);
                    KNGEOCOORD crdFlag10 = GetNextPoint(crdFlag7, fArrowAngle2, fFlagpoleL4);
                    KNGEOCOORD crdFlag11 = GetNextPoint(crdFlag9, fArrowAngle2, fFlagpoleL5);
                    KNGEOCOORD crdFlag12 = GetNextPoint(crdFlag10, fArrowAngle2, fFlagpoleL5);
                    KNGEOCOORD crdFlag13 = GetNextPoint(crdFlag5, fArrowAngle1, fFlagpoleL5);
                    KNGEOCOORD crdFlag14 = GetNextPoint(crdFlag7, fArrowAngle1, fFlagpoleL5);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdFlag13);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdFlag14);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdFlag12);
                    //m_ptShape[3] = m_ViewportManager.LogicalToDevice(crdFlag11);
                    //m_pSurfMain->Polygon(m_ptShape, 4, steeringColor);
                    convertPointUnit(crdFlag13.fLongitude, crdFlag13.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdFlag14.fLongitude, crdFlag14.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdFlag12.fLongitude, crdFlag12.fLatitude, m_tempPointBuff[2]);
                    convertPointUnit(crdFlag11.fLongitude, crdFlag11.fLatitude, m_tempPointBuff[3]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 4; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();

                    // 1. 箭头
                    kn_float fTriangleL1 = 1.2f;
                    kn_float fTriangleL2 = 0.2f;
                    KNGEOCOORD crdTri1((crdFlag9.ulLongitude + crdFlag10.ulLongitude) / 2, (crdFlag9.ulLatitude + crdFlag10.ulLatitude) / 2);
                    crdTri1.set((crdFlag9.fLongitude + crdFlag10.fLongitude) / 2.0f, (crdFlag9.fLatitude + crdFlag10.fLatitude) / 2.0f);
                    crdTri1 = GetNextPoint(crdTri1, fArrowAngle2, fTriangleL1);
                    KNGEOCOORD crdTri2 = GetNextPoint(crdFlag9, fArrowAngle4, fTriangleL2);
                    KNGEOCOORD crdTri3 = GetNextPoint(crdFlag10, fArrowAngle3, fTriangleL2);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdTri1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdTri2);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdTri3);
                    //m_pSurfMain->Polygon(m_ptShape, 3, steeringColor);
                    convertPointUnit(crdTri1.fLongitude, crdTri1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdTri2.fLongitude, crdTri2.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdTri3.fLongitude, crdTri3.fLatitude, m_tempPointBuff[2]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 3; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();
                }

                {
                    // 2. 旗杆
                    kn_float fFlagpoleL0 = 0.05f;
                    kn_float fFlagpoleL1 = 2.5f;
                    kn_float fFlagpoleL2 = 0.25f;
                    KNGEOCOORD crdFlag0 = GetNextPoint(ArrowCentrePt, fArrowAngle1, fFlagpoleL0);
                    KNGEOCOORD crdFlag1 = GetNextPoint(crdFlag0, fArrowAngle3, fFlagpoleL2);
                    KNGEOCOORD crdFlag2 = GetNextPoint(crdFlag0, fArrowAngle4, fFlagpoleL2);
                    KNGEOCOORD crdFlag3 = GetNextPoint(crdFlag2, fArrowAngle2, fFlagpoleL1);
                    KNGEOCOORD crdFlag4 = GetNextPoint(crdFlag1, fArrowAngle2, fFlagpoleL1);
                    //m_ptShape[0] = m_ViewportManager.LogicalToDevice(crdFlag1);
                    //m_ptShape[1] = m_ViewportManager.LogicalToDevice(crdFlag2);
                    //m_ptShape[2] = m_ViewportManager.LogicalToDevice(crdFlag3);
                    //m_ptShape[3] = m_ViewportManager.LogicalToDevice(crdFlag4);
                    //m_pSurfMain->Polygon(m_ptShape, 4, steeringColor);
                    convertPointUnit(crdFlag1.fLongitude, crdFlag1.fLatitude, m_tempPointBuff[0]);
                    convertPointUnit(crdFlag2.fLongitude, crdFlag2.fLatitude, m_tempPointBuff[1]);
                    convertPointUnit(crdFlag3.fLongitude, crdFlag3.fLatitude, m_tempPointBuff[2]);
                    convertPointUnit(crdFlag4.fLongitude, crdFlag4.fLatitude, m_tempPointBuff[3]);
                    glBegin(GL_POLYGON);
                    for (int i = 0; i < 4; i++) {
                        glVertex2d(m_tempPointBuff[i].x - m_viewWindow.left, m_tempPointBuff[i].y - m_viewWindow.bottom);
                    }
                    glEnd();
                }
            }
#endif
        }
    }
}

void DrivingMapWidget::drawHistoryTrace()
{
    HistoryTrace* pHistoryTrace = &(m_pNaviData->m_historyTrace);
     if (!pHistoryTrace->isValid()) {
         return;
     }

     kn_uint ii = 0;
     kn_uint iShapeSize = pHistoryTrace->m_historyTrace.size();

     if (iShapeSize > MAX_POINT_BUFF) {
         qDebug("DrivingMapWidget::drawHistoryTrace, iShapeSize(%d) too long", iShapeSize);
         QMessageBox::warning(this,
                              tr("Autonomous Driving Control"),
                              tr("Draw History Trace, shape size too long"),
                              QMessageBox::Yes);

         iShapeSize = MAX_POINT_BUFF;
     }
     if (0 == iShapeSize) {
         return;
     }

     MAP_RECTANGLE box;
     box.left = 999999999.0;
     box.right = -999999999.0;
     box.top = -999999999.0;
     box.bottom = 999999999.0;
     for (ii = 0; ii < iShapeSize; ii++) {
         convertPointUnit(pHistoryTrace->m_historyTrace[ii].Longitude,
                          pHistoryTrace->m_historyTrace[ii].Lattitude, m_tempPointBuff[ii]);

         box.left = box.left > m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.left;
         box.right = box.right < m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.right;
         box.top = box.top < m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.top;
         box.bottom = box.bottom > m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.bottom;
     }

     if (isClipedScreen(box)) {
         return;
     }

     qglColor(QColor(0, 255, 0));

     glBegin(GL_LINE_STRIP);
     for (ii = 0; ii < iShapeSize; ii++) {
         glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
     }
     glEnd();
}

void DrivingMapWidget::drawCurrentTrace()
{
    CurrentTrace* pCurrentTrace = &(m_pNaviData->m_currentTrace);
    if (!pCurrentTrace->isValid()) {
        return;
    }

    kn_uint ii = 0;
    kn_uint iShapeSize = pCurrentTrace->m_currentTrace.size();

    if (iShapeSize > MAX_POINT_BUFF) {
        qDebug("MapWidget::drawHistoryTrace, iShapeSize(%d) too long", iShapeSize);
        QMessageBox::warning(this,
                             tr("Autonomous Driving Control"),
                             tr("Draw Current Trace, shape size too long"),
                             QMessageBox::Yes);

        iShapeSize = MAX_POINT_BUFF;
    }
    if (0 == iShapeSize) {
        return;
    }

    MAP_RECTANGLE box;
    box.left = 999999999.0;
    box.right = -999999999.0;
    box.top = -999999999.0;
    box.bottom = 999999999.0;
    for (ii = 0; ii < iShapeSize; ii++) {
        convertPointUnit(pCurrentTrace->m_currentTrace[ii].Longitude,
                         pCurrentTrace->m_currentTrace[ii].Lattitude, m_tempPointBuff[ii]);

        box.left = box.left > m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.left;
        box.right = box.right < m_tempPointBuff[ii].x ? m_tempPointBuff[ii].x : box.right;
        box.top = box.top < m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.top;
        box.bottom = box.bottom > m_tempPointBuff[ii].y ? m_tempPointBuff[ii].y : box.bottom;
    }

    if (isClipedScreen(box)) {
        return;
    }

    qglColor(QColor(0, 0, 255));

    //glBegin(GL_LINE_STRIP);
    glPointSize(4);
    glBegin(GL_POINTS);
    for (ii = 0; ii < iShapeSize; ii++) {
        glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
    }
    glEnd();
}

void DrivingMapWidget::drawCar()
{
//    const static int halfCarWidth = 40;
//    const static int halfCarHeight = 100;

    // 左下
    m_tempPointBuff[0].x = m_currentPoint.x - m_halfCarWidth - 10;
    m_tempPointBuff[0].y = m_currentPoint.y - m_halfCarHeight;
    // 右下
    m_tempPointBuff[1].x = m_currentPoint.x + m_halfCarWidth + 10;
    m_tempPointBuff[1].y = m_currentPoint.y - m_halfCarHeight;
    // 右上
    m_tempPointBuff[2].x = m_currentPoint.x + m_halfCarWidth + 10;
    m_tempPointBuff[2].y = m_currentPoint.y + m_halfCarHeight;
    // 左上
    m_tempPointBuff[3].x = m_currentPoint.x - m_halfCarWidth - 10;
    m_tempPointBuff[3].y = m_currentPoint.y + m_halfCarHeight;

    if (0 == m_carIconTextureId) {
        qglColor(QColor(108, 117, 221));

        glBegin(GL_POLYGON);
        for (int ii = 0; ii < 4; ii++) {
            glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
        }
        glEnd();
    } else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_carIconTextureId);

        glBegin(GL_QUADS);
        {
            glColor3ub(255,255,255);    //清除背景颜色干扰

//            glTexCoord2d(0.0f, 0.0f);   // 左下
//            glVertex2d(m_tempPointBuff[0].x - m_viewWindow.left, m_tempPointBuff[0].y - m_viewWindow.bottom); // 左下

//            glTexCoord2d(1.0f, 0.0f);   // 右下
//            glVertex2d(m_tempPointBuff[1].x - m_viewWindow.left, m_tempPointBuff[1].y - m_viewWindow.bottom); // 右下

//            glTexCoord2d(1.0f,1.0f);   // 右上
//            glVertex2d(m_tempPointBuff[2].x - m_viewWindow.left, m_tempPointBuff[2].y - m_viewWindow.bottom); // 右上

//            glTexCoord2d(0.0f, 1.0f);   // 左上
//            glVertex2d(m_tempPointBuff[3].x - m_viewWindow.left, m_tempPointBuff[3].y - m_viewWindow.bottom); // 左上

            glTexCoord2d(0.0f, 0.0f);   // 左下
            glVertex2d(m_tempPointBuff[3].x - m_viewWindow.left, m_tempPointBuff[3].y - m_viewWindow.bottom); // 左上

            glTexCoord2d(1.0f, 0.0f);   // 右下
            glVertex2d(m_tempPointBuff[2].x - m_viewWindow.left, m_tempPointBuff[2].y - m_viewWindow.bottom); // 右上

            glTexCoord2d(1.0f,1.0f);   // 右上
            glVertex2d(m_tempPointBuff[1].x - m_viewWindow.left, m_tempPointBuff[1].y - m_viewWindow.bottom); // 右下

            glTexCoord2d(0.0f, 1.0f);   // 左上
            glVertex2d(m_tempPointBuff[0].x - m_viewWindow.left, m_tempPointBuff[0].y - m_viewWindow.bottom); // 左下
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }



    POINT_2D_ point;
    point.x = m_currentPoint.x- m_viewWindow.left;
    point.y = m_currentPoint.y- m_viewWindow.bottom;

    drawDetectZone(point);
}

void DrivingMapWidget::drawBgImg()
{
    if(m_naviDrvingImgTextureId)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_naviDrvingImgTextureId);

        glBegin(GL_QUADS);

        glColor3ub(255,255,255);    //清除背景颜色干扰

        glTexCoord2d(0.3f, 0.1f);   // 左下
        glVertex2d(0, height()); // 左下

        glTexCoord2d(0.8f, 0.1f);   // 右下
        glVertex2d(width(),height()); // 右下

        glTexCoord2d(0.8f, 0.7f);   // 右上
        glVertex2d(width(),0); // 右上

        glTexCoord2d(0.3f, 0.7f);   // 左上
        glVertex2d(0, 0); // 左上

        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
}

void DrivingMapWidget::drawObstacles()
{
    //
    glPointSize(6);

    //glBegin(GL_POINTS);
    // 后雷达
    for (int i = 0; i < BACK_RADAR_INFO::MAX_OBJ_COUNT; i++) {
        if (m_radarInfo.backLeftRadar[i].distance > 0.001) {
            if (m_radarInfo.backLeftRadar[i].distance > 2.0) {
                qglColor(QColor(0, 255, 0));
            } else {
                qglColor(QColor(255, 0, 0));
            }
//            glVertex2d(m_radarInfo.backLeftRadar[i].objPoint.x - m_viewWindow.left,
//                       m_radarInfo.backLeftRadar[i].objPoint.y - m_viewWindow.bottom);

            drawPoint(m_radarInfo.backLeftRadar[i].objPoint.x - m_viewWindow.left,
                      m_radarInfo.backLeftRadar[i].objPoint.y - m_viewWindow.bottom,
                      0,
                      meterToGeoCoord(0.5));
        }

        if (m_radarInfo.backRightRadar[i].distance > 0.001) {
            if (m_radarInfo.backRightRadar[i].distance > 2.0) {
                qglColor(QColor(0, 255, 0));
            } else {
                qglColor(QColor(255, 0, 0));
            }
//            glVertex2d(m_radarInfo.backRightRadar[i].objPoint.x - m_viewWindow.left,
//                       m_radarInfo.backRightRadar[i].objPoint.y - m_viewWindow.bottom);

            drawPoint(m_radarInfo.backRightRadar[i].objPoint.x - m_viewWindow.left,
                      m_radarInfo.backRightRadar[i].objPoint.y - m_viewWindow.bottom,
                      0,
                      meterToGeoCoord(0.5));
        }
    }

    // 前雷达
    for (int i = 0; i < FRONT_RADAR_INFO::MAX_OBJ_COUNT; ++i) {
        if (m_radarInfo.frontRadar[i].distance > 0.001) {
            if (m_radarInfo.frontRadar[i].distance > 10.0) {
                qglColor(QColor(0, 255, 0));
            } else {
                qglColor(QColor(255, 0, 0));
            }
//            glVertex2d(m_radarInfo.frontRadar[i].objPoint.x - m_viewWindow.left,
//                       m_radarInfo.frontRadar[i].objPoint.y - m_viewWindow.bottom);

            drawPoint(m_radarInfo.frontRadar[i].objPoint.x - m_viewWindow.left,
                      m_radarInfo.frontRadar[i].objPoint.y - m_viewWindow.bottom,
                      0,
                      meterToGeoCoord(0.5));
        }
    }

    // 前激光雷达
    for (int i = 0; i < IBEO_INFO::MAX_OBJECTS; ++i) {
        if (m_lidarInfo.frontLidar[i].distance > 0.001) {
            if (m_lidarInfo.frontLidar[i].distance > 10.0) {
                qglColor(QColor(0, 255, 0));
            } else {
                qglColor(QColor(255, 0, 0));
            }
            glBegin(GL_POINTS);
            glVertex2d(m_lidarInfo.frontLidar[i].objPoint.x - m_viewWindow.left,
                       m_lidarInfo.frontLidar[i].objPoint.y - m_viewWindow.bottom);
            glEnd();

            double boxWidth = meterToGeoCoord(m_lidarInfo.frontLidar[i].objInfo.boundingbox_y);
            double boxHeight = meterToGeoCoord(m_lidarInfo.frontLidar[i].objInfo.boundingbox_x);
            double x = m_lidarInfo.frontLidar[i].boundingCenterPoint.x - boxWidth / 2.0 - m_viewWindow.left;
            double y = m_lidarInfo.frontLidar[i].boundingCenterPoint.y - boxHeight / 2.0 - m_viewWindow.bottom;

            drawRect(x,y,0, boxWidth, boxHeight);
        }
    }

    //glEnd();

    clearRadarInfo();
}

void DrivingMapWidget::drawDrivingInfo()
{
//    if (m_drivingInfo.nearestDistance < m_carFrontFieldOfVision) {
//        qglColor(QColor(255, 0, 0));
//        glPointSize(4);
//        glBegin(GL_POINTS);
//        glVertex2d(m_drivingInfo.nearestPoint.x - m_viewWindow.left,
//                   m_drivingInfo.nearestPoint.y - m_viewWindow.bottom);
//        glEnd();
//    }
    //COM_DEBUG(5, ("ADC: neareast point[%f, %f]", m_drivingInfo.nearestPoint.x, m_drivingInfo.nearestPoint.y));
}

void DrivingMapWidget::drawCamInfo()
{
    // 左下
    m_tempPointBuff[0].x = m_camInfo.imgPos.leftBottom.x;
    m_tempPointBuff[0].y = m_camInfo.imgPos.leftBottom.y;
    // 右下
    m_tempPointBuff[1].x = m_camInfo.imgPos.rightBottom.x;
    m_tempPointBuff[1].y = m_camInfo.imgPos.rightBottom.y;
    // 右上
    m_tempPointBuff[2].x = m_camInfo.imgPos.rightTop.x;
    m_tempPointBuff[2].y = m_camInfo.imgPos.rightTop.y;
    // 左上
    m_tempPointBuff[3].x = m_camInfo.imgPos.leftTop.x;
    m_tempPointBuff[3].y = m_camInfo.imgPos.leftTop.y;

    MAP_RECTANGLE box;
    box.left = m_tempPointBuff[0].x;
    box.right = m_tempPointBuff[1].x;
    box.top = m_tempPointBuff[2].y;
    box.bottom = m_tempPointBuff[0].y;

    if (isClipedScreen(box)) {
        return;
    }

    if (m_camInfo.m_recoRetImgTextureId) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_camInfo.m_recoRetImgTextureId);

        glBegin(GL_QUADS);
        {
            glColor3ub(255,255,255);    //清除背景颜色干扰

            glTexCoord2d(0.0f, 0.0f);   // 左下
            glVertex2d(m_tempPointBuff[3].x - m_viewWindow.left, m_tempPointBuff[3].y - m_viewWindow.bottom); // 左上

            glTexCoord2d(1.0f, 0.0f);   // 右下
            glVertex2d(m_tempPointBuff[2].x - m_viewWindow.left, m_tempPointBuff[2].y - m_viewWindow.bottom); // 右上

            glTexCoord2d(1.0f,1.0f);   // 右上
            glVertex2d(m_tempPointBuff[1].x - m_viewWindow.left, m_tempPointBuff[1].y - m_viewWindow.bottom); // 右下

            glTexCoord2d(0.0f, 1.0f);   // 左上
            glVertex2d(m_tempPointBuff[0].x - m_viewWindow.left, m_tempPointBuff[0].y - m_viewWindow.bottom); // 左下
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
}

void DrivingMapWidget::drawMovementCtlInfo()
{
    const MovementControl::CTL_RET& ctlRet = m_movementCtl.getCtlResult();

    setFont(QFont("Times", 12));
    // 绘制坐标系
    double x1 = m_currentPoint.x - m_movementCtl.m_validControlDistance - m_viewWindow.left;
    double y1 = m_currentPoint.y - m_viewWindow.bottom;
    double x2 = m_currentPoint.x + m_movementCtl.m_validControlDistance - m_viewWindow.left;
    qglColor(QColor(255, 255, 255));
    glBegin(GL_LINES);
    glVertex2d(x1, y1);
    glVertex2d(x2, y1);
    glEnd();
    renderText(x2, y1, 0, "x");
    double x3 = m_currentPoint.x - m_viewWindow.left;
    double y3 = m_currentPoint.y - m_movementCtl.m_validControlDistance - m_viewWindow.bottom;
    double y4 = m_currentPoint.y + m_movementCtl.m_validControlDistance - m_viewWindow.bottom;
    qglColor(QColor(0, 0, 0));
    glBegin(GL_LINES);
    glVertex2d(x3, y3);
    glVertex2d(x3, y4);
    glEnd();
    renderText(x3, y4, 0, "y");

    // 绘制预瞄线
    qglColor(QColor(255, 0, 0));
    glBegin(GL_LINES);
    glVertex2d(m_currentPoint.x - m_viewWindow.left,
               m_currentPoint.y - m_viewWindow.bottom);
    glVertex2d(ctlRet.convergePoint.x - m_viewWindow.left,
               ctlRet.convergePoint.y - m_viewWindow.bottom);
    glEnd();

    // 绘制文本信息
    glPushMatrix(); // 文本不旋转
    glLoadIdentity();
    qglColor(QColor(255, 0, 0));
    char strBuff[64] = { 0 };
    double x = m_currentPoint.x + m_halfCarWidth + 20 - m_viewWindow.left;
    double y = m_currentPoint.y - m_viewWindow.bottom;
    sprintf(strBuff, "offset angle: %d", (int)ctlRet.offsetAngle);
    renderText(x, y, 0, strBuff);
    y -= 80;
    sprintf(strBuff, "wheel angle: %d", (int)ctlRet.wheelAngle);
    renderText(x, y, 0, strBuff);
    glPopMatrix();  // 文本不旋转
}

void DrivingMapWidget::drawDetectZone(POINT_2D_ origin)
{
    glShadeModel(GL_SMOOTH);

    //车头中心点
    POINT_2D_ point;
    point.x = origin.x;
    point.y = origin.y + m_halfCarHeight;

    //摄像头
    glColor4d(0,0,255,0.3);
    drawDetailZone(point,45.0,45.0,90.0,48.0);

    //前置雷达
    glColor4d(255,0,0,1);
    drawDetailZone(point,40.0,10.0,90.0,114.0);

    //车辆左下角
    point.x = origin.x - m_halfCarWidth;
    point.y = origin.y - m_halfCarHeight;

    glColor4d(255,0,0,0.8);
    drawDetailZone(point,10.0,2.0,250.0,60.0);

    //车辆右下角
    point.x = origin.x + m_halfCarWidth;
    point.y = origin.y - m_halfCarHeight;

    glColor4d(255,0,0,0.8);
    drawDetailZone(point,10.0,2.0,290.0,60.0);

    glShadeModel(GL_FLAT);
}

void DrivingMapWidget::drawDetailZone(POINT_2D_ origin, double sDis, double wDis, float sAngle, float nAngle)
{
    double R = meterToGeoCoord(wDis);

    //绘制警戒范围
    glBegin(GL_TRIANGLE_FAN);
    {
        glVertex2d(origin.x,origin.y);
        for (float i=sAngle-nAngle/2;!(i>sAngle+nAngle/2);i =i+nAngle/100.0)
        {
            glColor4d(255,255,255,0.1);

            glVertex2d(origin.x+R*cos(i*PI/180),origin.y+R*sin(i*PI/180));
        }
    }
    glEnd();

    //绘制安全区域
    glBegin(GL_QUAD_STRIP);
    {
        double R1 = meterToGeoCoord(wDis);
        double R2 = meterToGeoCoord(sDis);

        for (float i=sAngle-nAngle/2;!(i>sAngle+nAngle/2);i =i+nAngle/100.0)
        {
            glColor4d(0,255,0,0.5);
            glVertex2d(origin.x+R1*cos(i*PI/180),origin.y+R1*sin(i*PI/180));

            glColor4d(255,255,255,0.1);
            glVertex2d(origin.x+R2*cos(i*PI/180),origin.y+R2*sin(i*PI/180));
        }
    }
    glEnd();

    //绘制虚线
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1.0f,0x0F0F);

    R = meterToGeoCoord(sDis);

    glBegin(GL_LINES);
    {
        double angle;

        angle = sAngle;
        glColor3d(255,255,255);
        glVertex2d(origin.x,origin.y);
        glVertex2d(origin.x+R*cos(angle*PI/180),origin.y+R*sin(angle*PI/180));

        angle = sAngle-nAngle/2;
        glColor4d(0,0,0,0.2);
        glVertex2d(origin.x,origin.y);
        glVertex2d(origin.x+R*cos(angle*PI/180),origin.y+R*sin(angle*PI/180));

        angle = sAngle+nAngle/2;
        glVertex2d(origin.x,origin.y);
        glVertex2d(origin.x+R*cos(angle*PI/180),origin.y+R*sin(angle*PI/180));
    }
    glEnd();

    glDisable(GL_LINE_STIPPLE);

    //显示探测距离
//    QString strBuff;
//    strBuff.sprintf("%0.1f",sDis);
//    glColor3d(0,0,0);

//    double angle;
//    angle= sAngle-nAngle/2;
//    renderText(origin.x+R*cos(angle*PI/180),origin.y+R*sin(angle*PI/180),0,strBuff);

//    angle= sAngle+nAngle/2;
//    renderText(origin.x+R*cos(angle*PI/180),origin.y+R*sin(angle*PI/180),0,strBuff);
}

void DrivingMapWidget::drawPoint(double x, double y, double z, double r)
{
    drawCircle(x,y,z,r,GL_POLYGON);

    glColor4d(0,0,0,0.8);
    drawCircle(x,y,z,r,GL_LINE_LOOP);
}

void DrivingMapWidget::drawCircle(double x, double y, double z, double r, unsigned int type)
{
    glLineWidth(0.5);
    glBegin(type);
    {
        float angle = 0;

        for (int i = 0;i<50;i++)
        {
            angle = 2*PI*i/50;

            double vx = x + r*cos(angle);
            double vy = y + r*sin(angle);
            double vz = z;
            glVertex3d(vx,vy,vz);
        }
    }
    glEnd();
    glLineWidth(1.0);
}

void DrivingMapWidget::drawRect(double x, double y, double z, double w, double h)
{
    glLineWidth(0.5);
    glBegin(GL_LINE_LOOP);
    {
        glVertex3d(x,y,z);
        glVertex3d(x+w,y,z);
        glVertex3d(x+w,y+h,z);
        glVertex3d(x,y+h,z);
    }
    glEnd();
    glLineWidth(1.0);
}

void DrivingMapWidget::loadCarIcon()
{
    QImage* img2 = new QImage;

    if(! ( img2->load(":/image/car.png") ) ) {
            QMessageBox::information(this, tr("Auto Driving Control"), tr("Load car icon failed!"));
            delete img2;
            return;
    }

    //QImage* img = img2;//img2->scaled(64, 128);

    QImage img = img2->scaled(64, 128);
    delete img2;

    GLuint idTexture = 0;

    glGenTextures(1,&idTexture);
    if (idTexture)
    {
        glBindTexture(GL_TEXTURE_2D, idTexture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // 生成纹理
        glTexImage2D(GL_TEXTURE_2D,0,4,img.width(), img.height(),0,GL_BGRA_EXT,GL_UNSIGNED_BYTE,img.bits());
        COM_DEBUG(5, ("ADC: Car image width(%d) height(%d)", img.width(), img.height()));
    }

    m_carIconTextureId = idTexture;

    return;
}

void DrivingMapWidget::loadBgImg()
{
    QImage* img2 = new QImage;

    if(! ( img2->load(":/image/bg.png") ) ) {
            QMessageBox::information(this, tr("Auto Driving Control"), tr("Load background icon failed!"));
            delete img2;
            return;
    }

    QImage img = img2->scaled(img2->width(), img2->height());
    delete img2;

    GLuint idTexture = 0;

    glGenTextures(1,&idTexture);
    if (idTexture)
    {
        glBindTexture(GL_TEXTURE_2D, idTexture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // 生成纹理
        glTexImage2D(GL_TEXTURE_2D,0,4,img.width(), img.height(),0,GL_BGRA_EXT,GL_UNSIGNED_BYTE,img.bits());
        COM_DEBUG(5, ("ADC: background image width(%d) height(%d)", img.width(), img.height()));
    }
    m_naviDrvingImgTextureId = idTexture;
    return;
}

void DrivingMapWidget::clearRadarInfo()
{
    memset(&m_radarInfo, 0, sizeof(m_radarInfo));
    memset(&m_lidarInfo, 0, sizeof(m_lidarInfo));
}
