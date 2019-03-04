#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include "systemlog.h"

class MyApplication:public QApplication
{
    Q_OBJECT
public:
    MyApplication(int &argc,char **argv);
    ~MyApplication(){}
protected:
    bool winEventFilter(MSG *message, long *result);
signals:
    void MsgCamera_WM_LANENUMBER(WPARAM wParam,LPARAM lParam);

    void MsgCamera_WM_LANEDISTANCE_LD_RD_X(const QString & strWParam);
    void MsgCamera_WM_LANEDISTANCE_LD_RD_Y(const QString & strLParam);
    void MsgCamera_WM_LANECHANGE_LEFT_RIGHT(int iLane);

    void MsgCamera_WM_TRAFFICSIGNAL_XYZ_L(const QString & strWParam);
    void MsgCamera_WM_TRAFFICSIGNAL_XYZ_R(const QString & strLParam);
    void MsgCamera_WM_TRAFFICSIGNAL_TYPE(int iType);
    void MsgMobileNavigator_VERSION(const QString & strLParam);
    void MsgMobileNavigator_EnterPark();
    void MsgMobileNavigator_LeavePark();
    void MsgCameraCAN_VERSION(const QString & strLParam);
    void MsgCamera_WM_CHANGEROAD(int iDirection);
private:
    int m_changeRoadSwich;
};

class CProControlIPCThread:public QThread
{
protected:
    void run();
};

extern QString g_logPath;
extern MyApplication * g_strMyApplication;


#endif // MAIN_H
