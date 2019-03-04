#ifndef ROADSCREEN_H
#define ROADSCREEN_H

#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <windows.h>
#include "BaseDefine.h"

class RoadScreen : public QWidget
{
    Q_OBJECT
public:
    explicit RoadScreen(QWidget* parent = 0);
    ~RoadScreen();
    void changeCarInRoad(int nIndex);

signals:
    /*******20150120 bootmanager send event to navi (begin)********/
    void clicked(int area);
    /*******20150120 bootmanager send event to navi (end)********/


public slots:
    //    void onAdjustTimerOut(void);
    void onChangeRoadScreen(WPARAM nRoadCount, LPARAM nRoadIndex);
    /*******20150120 bootmanager send event to navi (begin)********/
    //void onOpenChangeRoad();
    //void onCloseChangeRoad();
    void handleChangeRoad();
    void sendRoadMessage(int roadIndex);
    /*******20150120 bootmanager send event to navi (end)********/

    /*********20150306(begin)***********/
    void sendRoadMessageByKey(int direction);
    /*********20150306(end)*************/



protected:
    void paintEvent(QPaintEvent*);


private:
    QLabel* m_pLabel;
    QImage* m_pImage;

    int m_nRoadCount;
    int m_nRoadIndex;

    QPoint m_pFrontOne;   // point 1
    QPoint m_pFrontTwo;   // point 2
    QPoint m_pFrontThree;   // point 3
    QPoint m_pFrontFour;   // point 4

    QPoint m_pBackOne;   // left and up
    QPoint m_pBackTwo;   // right and up
    QPoint m_pBackThree;   // left and down
    QPoint m_pBackFour;   // right and down

    qreal m_nCarX;
    qreal m_nCarY;

    //    QTimer* m_pTimer;
    /*******20150120 bootmanager send event to navi (begin)********/
    //BOOL roadChangeFlag;
    int m_roadChangeFlag;
    /*******20150120 bootmanager send event to navi (end)********/

protected:
    void mousePressEvent(QMouseEvent* event);
    //void mouseMoveEvent(QMouseEvent *event);
    QPoint dragPosition;
};

#endif // ROADSCREEN_H
