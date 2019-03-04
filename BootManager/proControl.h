#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "ControlPage.h"
extern int navi_status;

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
QT_END_NAMESPACE

class CProControl : public QWidget
{
    Q_OBJECT
    
public:
    CProControl(QWidget *parent = 0);
    ~CProControl();

signals:
    //void notifyCloseAll(void);
    void notifyCloseNavi(void);
    void notifyCloseCam(void);


public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void closeAll();

private:
    void createIcons();

    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;



    uchar m_ucDebugType;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    QPoint dragPosition;
};

#endif // DIALOG_H
