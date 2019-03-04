#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QtWidgets>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QLabel>


class NaviWidget;
class GpsWidget;
////class RecvOutSideMsg;
class CanWidget;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void configFunctions(void);
private:
    QListWidget* m_pWidgetSelector;
    QStackedWidget* m_pWidgets;

    NaviWidget* m_pNaviWidget;
    GpsWidget* m_pGpsWidget;
    CanWidget* m_pCanWidget;
    QLabel* m_pDummyLabel1;
    QLabel* m_pDummyLabel2;
    QLabel* m_pDummyLabel3;
    QLabel* m_pDummyLabel4;

    ////RecvOutSideMsg* m_pRecvOutsideMsg;
protected:
    void paintEvent(QPaintEvent *);
};

#endif // MAINWINDOW_H
