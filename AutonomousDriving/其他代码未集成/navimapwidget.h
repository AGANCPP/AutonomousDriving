#ifndef NAVIMAPWIDGET_H
#define NAVIMAPWIDGET_H

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QtOpenGL/QGLWidget>

#include "navidata.h"

/******************************************************************************
 * class define
 ******************************************************************************/
class QRadialGradient;
class QPainter;
class QToolButton;
class QMenuBar;
class QMenu;
class QAction;
class QGLFramebufferObject;
class NaviData;

class MapWidget : public QGLWidget
{
    Q_OBJECT

public:
    MapWidget(NaviData *pNaviData, QWidget* parent = 0);
    ~MapWidget();

    void setNaviData(NaviData* pNaviData) { m_pNaviData = pNaviData; }
    void reqUpdateMapFrame(void);
    void reqUpdateHistoryTraceFrame(void);
    void reqUpdateCurrentTraceFrame(void);

public slots:
    void dummySlot(void);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    void reshape();

    // 绘制控件
    void drawInternalWidgets();

    // 用于窗口控制
    void calculateDefaultViewArea();
    bool isClipedScreen(MAP_RECTANGLE& rect);   // 判断场景中的图元是否在视口内
    void recoverDefaultViewWindow();            // 恢复默认的显示窗口
    void mapFrameChanged();
    void currentTraceFrameChanged();
    void updateViewWindow();

    // 绘制地图
    void paintMap();
    void paintCurrentTrace();
    void drawMap();
    void drawCurrentTrace();
    void draw3DHighAccuracyData();
    void drawLaneCentral();
    void drawDummyLine();
    void drawRoadLaneLineHAD();
    void drawRoadAreaHAD();
    void drawPaintMark();
    void drawHistoryTrace();
    void drawRoadVectorBoundingBox();

    void loadBgImg();
    void drawBgImg();
    void paintBgImg();

private:
//    QToolButton* m_pChangeToMapViewButton;
//    QToolButton* m_pChangeToDrivingViewButton;

    // 导航相关数据
    NaviData* m_pNaviData;
    // 将地图作为一个图层进行显示
    //QGLFramebufferObject* m_pMapFrame;
    //QGLFramebufferObject* m_pCurrentTraceFrame;
    GLuint m_nMapGlShowListId;
    GLuint m_nCurrentTraceGlShowListId;
    // 用于地图缩放
    GLfloat m_fScaling;                         // 场景放大比例
    float m_fMapScale;                          // 场景与屏幕的比例
    // 用于地图移动
    QPoint m_lastPos;                           // 记录鼠标的位置
    GLdouble m_moveDx;                          // 记录移动的位置
    GLdouble m_moveDy;                          // 记录移动的位置
    // 用于控制显示地图的窗口
    MAP_RECTANGLE m_defaultViewArea;
    MAP_RECTANGLE m_viewWindow;                 // 记录视口的窗口在场景中的位置
    enum { MAX_POINT_BUFF = 30720 };
    MAP_POINT m_tempPointBuff[MAX_POINT_BUFF];  // 临时保存转换后的坐标

    GLuint m_naviDrvingImgTextureId;

};

#endif // NAVIMAPWIDGET_H
