// navimapwidget.cpp

/******************************************************************************
 * Head files
 ******************************************************************************/
#include <QtGui>
#include <QGLFramebufferObject>

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string>

#include "navimapwidget.h"
#include "commondebug.h"
#include "IHighAccuracyDataReader.h"

/******************************************************************************
 * class
 ******************************************************************************/
MapWidget::MapWidget(NaviData* pNaviData, QWidget* parent) : QGLWidget(parent)
{
    m_pNaviData = pNaviData;
    //this->setMinimumSize(QSize(200, 200));

//    // Create tool button
//    m_pChangeToMapViewButton = new QToolButton(this);
//    //m_pChangeToMapViewButton->setIcon(QIcon(":/image/icon_dummy.jpg"));
//    m_pChangeToMapViewButton->setText(tr("  Map  "));
//    m_pChangeToMapViewButton->adjustSize();
//    connect(m_pChangeToMapViewButton, SIGNAL(clicked()), this, SLOT(dummySlot()));

//    m_pChangeToDrivingViewButton = new QToolButton(this);
//    //m_pChangeToDrivingViewButton->setIcon(QIcon(":/image/icon_dummy.jpg"));
//    m_pChangeToDrivingViewButton->setText(tr("Driving"));
//    m_pChangeToDrivingViewButton->adjustSize();
//    connect(m_pChangeToDrivingViewButton, SIGNAL(clicked()), this, SLOT(dummySlot()));

    // 地图相关变量初始化
    makeCurrent();  // 如果不调用此函数，那么"new QGLFramebufferObject(1024, 1024)"将会失败
    // It is important to have a current GL context when creating a QGLFramebufferObject,
    // otherwise initialization will fail.
    //获取设备屏幕大小
    //QDesktopWidget* desktopWidget = QApplication::desktop();
    //QRect screenRect = desktopWidget->screenGeometry();
    //COM_DEBUG(5, ("screen rect (%d, %d)", screenRect.width(), screenRect.height()));
    //m_pMapFrame = new QGLFramebufferObject(screenRect.width(), screenRect.height());
    //m_pCurrentTraceFrame = new QGLFramebufferObject(screenRect.width(), screenRect.height());
    m_nMapGlShowListId = 0;
    m_nCurrentTraceGlShowListId = 0;

    m_fMapScale = 1.0;
    m_fScaling = 1.0;

    m_moveDx = 0.0;
    m_moveDy = 0.0;

    m_viewWindow.left = 0;
    m_viewWindow.bottom = 0;
    m_viewWindow.right = width();
    m_viewWindow.top = height();

    calculateDefaultViewArea();

    m_naviDrvingImgTextureId = 0;
    loadBgImg();
}

MapWidget::~MapWidget()
{
//    makeCurrent();
//    delete m_pMapFrame;
//    delete m_pCurrentTraceFrame;
}

void MapWidget::reqUpdateMapFrame(void)
{
    calculateDefaultViewArea();
    // Draw map
    recoverDefaultViewWindow();
    reshape();
    update();
}

void MapWidget::reqUpdateHistoryTraceFrame(void)
{
    calculateDefaultViewArea();
    // Draw map
    recoverDefaultViewWindow();
    reshape();
    update();
}

void MapWidget::reqUpdateCurrentTraceFrame(void)
{
    CurrentTrace& currentTrace = m_pNaviData->m_currentTrace;
    if (1 == currentTrace.m_currentTrace.size()) {
        currentTrace.calculateArea();
        if (isClipedScreen(currentTrace.m_area)) {
            calculateDefaultViewArea();
            recoverDefaultViewWindow();
            reshape();
            update();
            return;
        }
    }
    currentTraceFrameChanged();
    update();
}

void MapWidget::initializeGL()
{
    qglClearColor(QColor(210, 210, 210, 0));
    //qglClearColor(QColor(231, 231, 181, 0));
    glShadeModel(GL_FLAT);

    //开启抗锯齿
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MapWidget::resizeGL(int w, int h)
{
    //qDebug("MapWidget::resizeGL");

    drawInternalWidgets();

    reshape();
}

void MapWidget::reshape()
{
    // 更新显示区域
    updateViewWindow();

    mapFrameChanged();
    currentTraceFrameChanged();
}

void MapWidget::paintGL()
{
    //qDebug("MapWidget::paintGL");
//    glViewport(0, 0, width(), height());
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

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

    glLoadIdentity();

    paintMap();
    paintCurrentTrace();
}

void MapWidget::wheelEvent(QWheelEvent *event)
{
    double numDegrees = -event->delta() / 8.0;
    double numSteps = numDegrees / 15.0;
    m_fScaling *= std::pow(1.125, numSteps);
    //qDebug("MapWidget::wheelEvent m_fScaling=%f", m_fScaling);
    reshape();
    updateGL();
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastPos = event->pos();
        //qDebug("MapWidget::mousePressEvent screen (%d,%d)", event->pos().x(), event->pos().y());
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    GLfloat dx = GLfloat(event->x() - m_lastPos.x());
    GLfloat dy = GLfloat(event->y() - m_lastPos.y());
    //qDebug("MapWidget::mouseMoveEvent screen (%f,%f)", dx, dy);
    if (event->buttons() & Qt::LeftButton) {
        m_moveDx -= dx * m_fMapScale;
        m_moveDy += dy * m_fMapScale;
        //qDebug("MapWidget::mouseMoveEvent map (%f,%f)", m_moveDx, m_moveDy);
        reshape();
        updateGL();
    }
    m_lastPos = event->pos();
}

void MapWidget::dummySlot(void)
{
    qDebug("MapWidget::dummySlot");
}

void MapWidget::drawInternalWidgets()
{
//    int x1 = width() / 2 - (m_pChangeToMapViewButton->width());
//    int x2 = width() / 2;
//    //int x3 = width() - (m_pNaviMenuBar->width() + 10);
//    m_pChangeToMapViewButton->move(x1, 5);
//    m_pChangeToDrivingViewButton->move(x2, 5);
//    //m_pNaviMenuBar->move(x3, 5);
}

void MapWidget::paintMap()
{
    if ( (!(m_pNaviData->m_mapData.isMapValid())) &&
         (!(m_pNaviData->m_historyTrace.isValid())) ) {
        return;
    }

//    glEnable(GL_TEXTURE_2D);
//    glBindTexture(GL_TEXTURE_2D, m_pMapFrame->texture());
//    qglColor(QColor(255, 255, 255));
//    GLfloat s = width() / GLfloat(m_pMapFrame->size().width());
//    GLfloat t = height() / GLfloat(m_pMapFrame->size().height());

//    glBegin(GL_QUADS);
//    glTexCoord2f(0.0, 0.0);
//    glVertex2f(-1.0, -1.0);
//    glTexCoord2f(s, 0.0);
//    glVertex2f(1.0, -1.0);
//    glTexCoord2f(s, t);
//    glVertex2f(1.0, 1.0);
//    glTexCoord2f(0.0, t);
//    glVertex2f(-1.0, 1.0);
//    glEnd();

    //COM_DEBUG(5, ("MapWidget::paintMap"));

    //glCallList(m_nMapGlShowListId);
    drawMap();
}

void MapWidget::paintCurrentTrace()
{
    if (!(m_pNaviData->m_currentTrace.isValid())) {
        return;
    }

//    glEnable(GL_TEXTURE_2D);
//    glBindTexture(GL_TEXTURE_2D, m_pCurrentTraceFrame->texture());
//    qglColor(QColor(255, 255, 255));
//    GLfloat s = width() / GLfloat(m_pCurrentTraceFrame->size().width());
//    GLfloat t = height() / GLfloat(m_pCurrentTraceFrame->size().height());

//    glBegin(GL_QUADS);
//    glTexCoord2f(0.0, 0.0);
//    glVertex2f(-1.0, -1.0);
//    glTexCoord2f(s, 0.0);
//    glVertex2f(1.0, -1.0);
//    glTexCoord2f(s, t);
//    glVertex2f(1.0, 1.0);
//    glTexCoord2f(0.0, t);
//    glVertex2f(-1.0, 1.0);
//    glEnd();

    //glCallList(m_nCurrentTraceGlShowListId);
    drawCurrentTrace();
}

void MapWidget::recoverDefaultViewWindow()
{
    m_fScaling = 1.0;

    m_moveDx = 0.0;
    m_moveDy = 0.0;
}

void MapWidget::updateViewWindow()
{
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
}

void MapWidget::calculateDefaultViewArea()
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
    if (m_pNaviData->m_currentTrace.isValid()) {
        m_pNaviData->m_currentTrace.calculateArea();
        areas.push_back(m_pNaviData->m_currentTrace.m_area);
    }

    constructBoundingBox(areas, m_defaultViewArea);
    if ((m_defaultViewArea.right - m_defaultViewArea.left) < width()) {
        m_defaultViewArea.right = m_defaultViewArea.left + width();
        m_defaultViewArea.top = m_defaultViewArea.bottom + height();
    }

    qDebug("m_defaultViewArea[(%f, %f) (%f, %f)]",
           m_defaultViewArea.left, m_defaultViewArea.bottom, m_defaultViewArea.right, m_defaultViewArea.top);
}

bool MapWidget::isClipedScreen(MAP_RECTANGLE& rect)
{
//    qDebug("window[(%f, %f, %f, %f)] rect[(%f, %f, %f, %f)]",
//           m_viewWindow.left, m_viewWindow.bottom, m_viewWindow.right, m_viewWindow.top,
//           rect.left, rect.bottom, rect.right, rect.top);
    if( (m_viewWindow.left   > rect.right) ||
        (m_viewWindow.right  < rect.left) ||
        (m_viewWindow.top    < rect.bottom) ||
        (m_viewWindow.bottom > rect.top)) {
        // 图形不在视口内
        return true;
    }

    // 图形在视口内
    return false;
}

void MapWidget::mapFrameChanged()
{
//    if ( (!(m_pNaviData->m_mapData.isMapValid())) &&
//         (!(m_pNaviData->m_historyTrace.isValid())) ) {
//        return;
//    }

//    if (false == m_pMapFrame->bind()) {
//        COM_ERR("[ERR] MapWidget::mapFrameChanged bind() error");
//        return;
//    }

//    glDisable(GL_TEXTURE_2D);
//    // 定义视口
//    glViewport(0, 0, width(), height());
//    // 投影变换
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    // 定义正投影(裁剪区域)
//    //gluOrtho2D(/*left*/, /*right*/, /*bottom*/, /*top*/);
//    gluOrtho2D(0, m_viewWindow.right - m_viewWindow.left, 0, m_viewWindow.top - m_viewWindow.bottom);
//    // 在执行模型或视图变换之前，必须以GL_MODELVIEW为参数调用glMatrixMode()函数
//    glMatrixMode(GL_MODELVIEW);

//    // 绘制地图
//    glClear(GL_COLOR_BUFFER_BIT);
//    drawMap();

//    m_pMapFrame->release();

//    if (0 != m_nMapGlShowListId) {
//        glDeleteLists(m_nMapGlShowListId, 1);
//    }

//    m_nMapGlShowListId = glGenLists(1);
//    if (0 == m_nMapGlShowListId) {
//        COM_ERR("[ERR] ADC, %s glGenLists error", __FUNCTION__);
//        return;
//    }

//    COM_DEBUG(5, ("MapWidget::drawMap"));
//    glNewList(m_nMapGlShowListId, GL_COMPILE);
//    // 绘制地图
//    drawMap();
//    glEndList();
}

void MapWidget::currentTraceFrameChanged()
{
//    if (!(m_pNaviData->m_currentTrace.isValid())) {
//        return;
//    }

//    if (false == m_pCurrentTraceFrame->bind()) {
//        COM_ERR("[ERR] MapWidget::currentTraceFrameChanged bind() error");
//        return;
//    }

//    glDisable(GL_TEXTURE_2D);

//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    // 定义视口
//    glViewport(0, 0, width(), height());
//    // 投影变换
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    // 定义正投影(裁剪区域)
//    //gluOrtho2D(/*left*/, /*right*/, /*bottom*/, /*top*/);
//    gluOrtho2D(0, m_viewWindow.right - m_viewWindow.left, 0, m_viewWindow.top - m_viewWindow.bottom);
//    // 在执行模型或视图变换之前，必须以GL_MODELVIEW为参数调用glMatrixMode()函数
//    glMatrixMode(GL_MODELVIEW);

//    // 绘制地图
//    glClear(GL_COLOR_BUFFER_BIT);
//    drawCurrentTrace();

//    m_pCurrentTraceFrame->release();

//    if (0 != m_nCurrentTraceGlShowListId) {
//        glDeleteLists(m_nCurrentTraceGlShowListId, 1);
//    }

//    m_nCurrentTraceGlShowListId = glGenLists(1);
//    if (0 == m_nCurrentTraceGlShowListId) {
//        COM_ERR("[ERR] ADC, %s glGenLists error", __FUNCTION__);
//        return;
//    }

//    //COM_DEBUG(5, ("drawCurrentTrace"));
//    glNewList(m_nCurrentTraceGlShowListId, GL_COMPILE);
//    // 绘制地图
//    drawCurrentTrace();
//    glEndList();
}

void MapWidget::drawMap()
{
    draw3DHighAccuracyData();
    drawHistoryTrace();
}

void MapWidget::draw3DHighAccuracyData()
{
    drawPaintMark();
    drawRoadAreaHAD();
    //drawLaneCentral();
    //drawDummyLine();
    drawRoadLaneLineHAD();
    //drawRoadVectorBoundingBox();
}

void MapWidget::drawLaneCentral()
{
    //qDebug("MapWidget::drawLaneCentral");

    //KHAData* pHAData = m_pMapData->m_pHAData;
    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    if (NULL == pHAData) {
        return;
    }

    kn_dword dwLaneVecID = 0;
    kn_dword dwSEQ = 0;
    kn_dword dwCentralLaneCount = 0;

    //qDebug("LaneSections size: %d", pHAData->LaneSections().size());

    //qglColor(QColor(220, 125, 220));
    qglColor(QColor(250, 200, 230));

    glLineStipple(3, 0x3F07);
    glEnable(GL_LINE_STIPPLE);

    // 遍历 Section
    KHI_LANE_SECTION_LIST::const_iterator its_section = pHAData->LaneSections().begin();
    KHI_LANE_SECTION_LIST::const_iterator ite_section = pHAData->LaneSections().end();
    for (; its_section != ite_section; ++its_section) {
        const KHI_LANE_SECTION* pSection = (*its_section);

        dwLaneVecID         = pSection->dwRoadVecID;
        dwSEQ               = pSection->dwSEQ;
        dwCentralLaneCount  = pSection->dwCentralLaneCount;

        // 在车道中心线集合中查找 LANE_CENTRAL
        {
            KHI_LANE_CENTRAL_LIST listCentral = pHAData->LaneCentrals(KHI_LANE_CENTRAL::Help_Key(dwLaneVecID, dwSEQ));
            KHI_LANE_CENTRAL_LIST::const_iterator its_central = listCentral.begin();
            KHI_LANE_CENTRAL_LIST::const_iterator ite_central = listCentral.end();
            for ( ; its_central != ite_central; ++its_central) {
                const KHI_LANE_CENTRAL* pCentral = (*its_central);
                // 找到对应的车道中心线
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
                    }
                    glEnd();
                }
            }
        }
    }

    glDisable(GL_LINE_STIPPLE);
}

void MapWidget::drawDummyLine()
{
    //qDebug("MapWidget::drawDummyLine");

    //KHAData* pHAData = m_pMapData->m_pHAData;
    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    if (NULL == pHAData) {
        return;
    }

    //qglColor(QColor(0, 0, 255));
    qglColor(QColor(170, 180, 240));

    glLineStipple(3, 0x3F07);
    glEnable(GL_LINE_STIPPLE);

    KHI_DUMMY_LINE_LIST::const_iterator its_dummy = pHAData->DummyLines().begin();
    KHI_DUMMY_LINE_LIST::const_iterator ite_dummy = pHAData->DummyLines().end();

    for ( ; its_dummy != ite_dummy; ++its_dummy)
    {
        const KHI_DUMMY_LINE* pDummy = (*its_dummy);

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

    glDisable(GL_LINE_STIPPLE);
}

void MapWidget::drawRoadLaneLineHAD()
{
    //qDebug("MapWidget::drawRoadLaneLineHAD");

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
            qDebug("MapWidget::drawRoadLaneLineHAD, iShapeSize(%d) too long", iShapeSize);
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

void MapWidget::drawRoadAreaHAD()
{
    //qDebug("MapWidget::drawRoadAreaHAD");

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
            qDebug("MapWidget::drawRoadAreaHAD, iShapeSize(%d) too long", iShapeSize);
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

void MapWidget::drawPaintMark()
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

void MapWidget::drawHistoryTrace()
{
    HistoryTrace* pHistoryTrace = &(m_pNaviData->m_historyTrace);
    if (!pHistoryTrace->isValid()) {
        return;
    }

    kn_uint ii = 0;
    kn_uint iShapeSize = pHistoryTrace->m_historyTrace.size();

    if (iShapeSize > MAX_POINT_BUFF) {
        qDebug("MapWidget::drawHistoryTrace, iShapeSize(%d) too long", iShapeSize);
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

void MapWidget::drawCurrentTrace()
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
        if (ii == iShapeSize - 1) {
            qglColor(QColor(255, 0, 0));
        }
        glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
    }
    glEnd();
}

void MapWidget::drawRoadVectorBoundingBox()
{
    if ( (!(m_pNaviData->m_mapData.isMapValid())) &&
         (!(m_pNaviData->m_historyTrace.isValid())) ) {
        return;
    }

    //KHAData* pHAData = m_pHAData;
    KHAData* pHAData = m_pNaviData->m_mapData.m_pHAData;
    if (NULL != pHAData) {
        KHI_ROAD_VECTOR_LIST::const_iterator its_roadVector = pHAData->RoadVectors().begin();
        KHI_ROAD_VECTOR_LIST::const_iterator ite_roadVector = pHAData->RoadVectors().end();
        for (; its_roadVector != ite_roadVector; ++its_roadVector) {
            const KHI_ROAD_VECTOR* pRoadVector = (*its_roadVector);

            kn_uint ii = 0;
            kn_uint iShapeSize = 4;
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
            qglColor(QColor(255, 0, 0));
            glBegin(GL_LINE_LOOP);
            for (ii = 0; ii < iShapeSize; ii++) {
                glVertex2d(m_tempPointBuff[ii].x - m_viewWindow.left, m_tempPointBuff[ii].y - m_viewWindow.bottom);
            }
            glEnd();
        }
    }
}

void MapWidget::drawBgImg()
{
    if(m_naviDrvingImgTextureId)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_naviDrvingImgTextureId);

        glBegin(GL_QUADS);

        glColor3ub(255,255,255);    //清除背景颜色干扰

//        glTexCoord2d(0.5f, 0.2f);   // 左下
//        glVertex2d(0, height()); // 左下

//        glTexCoord2d(1.0f, 0.2f);   // 右下
//        glVertex2d(width(),height()); // 右下

//        glTexCoord2d(1.0f, 1.0f);   // 右上
//        glVertex2d(width(),0); // 右上

//        glTexCoord2d(0.5f, 1.0f);   // 左上
//        glVertex2d(0, 0); // 左上

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

void MapWidget::loadBgImg()
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
       COM_DEBUG(5, ("ADC: background image1 width(%d) height(%d)", img.width(), img.height()));
   }

   m_naviDrvingImgTextureId = idTexture;

   return;
}

void MapWidget::paintBgImg()
{
    drawBgImg();
}
