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

    // ���ƿؼ�
    void drawInternalWidgets();

    // ���ڴ��ڿ���
    void calculateDefaultViewArea();
    bool isClipedScreen(MAP_RECTANGLE& rect);   // �жϳ����е�ͼԪ�Ƿ����ӿ���
    void recoverDefaultViewWindow();            // �ָ�Ĭ�ϵ���ʾ����
    void mapFrameChanged();
    void currentTraceFrameChanged();
    void updateViewWindow();

    // ���Ƶ�ͼ
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

    // �����������
    NaviData* m_pNaviData;
    // ����ͼ��Ϊһ��ͼ�������ʾ
    //QGLFramebufferObject* m_pMapFrame;
    //QGLFramebufferObject* m_pCurrentTraceFrame;
    GLuint m_nMapGlShowListId;
    GLuint m_nCurrentTraceGlShowListId;
    // ���ڵ�ͼ����
    GLfloat m_fScaling;                         // �����Ŵ����
    float m_fMapScale;                          // ��������Ļ�ı���
    // ���ڵ�ͼ�ƶ�
    QPoint m_lastPos;                           // ��¼����λ��
    GLdouble m_moveDx;                          // ��¼�ƶ���λ��
    GLdouble m_moveDy;                          // ��¼�ƶ���λ��
    // ���ڿ�����ʾ��ͼ�Ĵ���
    MAP_RECTANGLE m_defaultViewArea;
    MAP_RECTANGLE m_viewWindow;                 // ��¼�ӿڵĴ����ڳ����е�λ��
    enum { MAX_POINT_BUFF = 30720 };
    MAP_POINT m_tempPointBuff[MAX_POINT_BUFF];  // ��ʱ����ת���������

    GLuint m_naviDrvingImgTextureId;

};

#endif // NAVIMAPWIDGET_H
