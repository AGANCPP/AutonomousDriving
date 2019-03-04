

#include <QtWidgets/QApplication>
#include <qfileinfo.h>
#include "MyApplication.h"
#include "systemlog.h"
////#include "IpcAccess.h"
#include "BaseDefine.h"
#include "proControl.h"
#include "rootviewer.h"
#include "wchar.h"


MyApplication * g_strMyApplication = NULL;

////int main(int argc, char *argv[])
////{
////    
////    g_strMyApplication = new MyApplication(argc, argv);
////
////    //BootManager apply singleton designing module
////    QSharedMemory sharedMemory("UniqueNameForApplication");
////    if(sharedMemory.attach())
////    {
////        //This message box statement must be put after the creation of the MyApplication's instance
////        QMessageBox::information(NULL,"Warning","BootManager is already opened.");
////        return -1;
////    }
////    sharedMemory.create(1);
////
////    //Install log message handler;Log file name is based on the current datetime.
////    //setCurrentLogFileName();
////    currentLogFileInit();
////    qInstallMsgHandler(customMessageHandler);
////
////    // Create com memery
////    CreateIpcComm();
////
////    //Read config file for displaying
////    int nMainProWidth = CONFIG_DEF_MAIN_WIDTH;
////    int nMainProHeight = CONFIG_DEF_MAIN_HEIGHT;
////    int nScreenWidth = CONFIG_DEF_SCR_WIDTH;
////    int nScreenHeight = CONFIG_DEF_SCR_HEIGHT;
////
////    QFileInfo fConfigIniFileInfo;
////    fConfigIniFileInfo.setFile(CONFIG_INI_FILE);
////    if(fConfigIniFileInfo.exists())
////    {
////        qDebug() << "config file exists, use config file value:";
////        QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
////        nMainProWidth  = iniSettings.value("Main/MainWidth").toInt();
////        nMainProHeight = iniSettings.value("Main/MainHeight").toInt();
////        nScreenWidth   = iniSettings.value("Screen/ScreenWidth").toInt();
////        nScreenHeight  = iniSettings.value("Screen/ScreenHeight").toInt();
////    }
////    else //use default value
////    {
////        qWarning() << "config file not exist, use default value.";
////    }
////
////    //Get the current screen resolution
////    QDesktopWidget* desktopWidget = QApplication::desktop();
////    QRect screenRect = desktopWidget->screenGeometry();
////
////    float m_nScaleW = (float)(screenRect.width())/(float)(nScreenWidth);
////    float m_nScaleH = (float)(screenRect.height())/(float)(nScreenHeight);
////
////    g_pViewer = new CProControl();
////    g_pViewer->setBaseSize((int)(m_nScaleW*nMainProWidth), (int)(m_nScaleH*nMainProHeight));
////    g_pViewer->setFixedSize((int)(m_nScaleW*nMainProWidth),(int)(m_nScaleH*nMainProHeight));
////    g_pViewer->move(screenRect.width() - g_pViewer->baseSize().width(),screenRect.height() - g_pViewer->baseSize().height());
////    g_pViewer->show();
////
////    /****20150306(begin)**********/
////    HWND hBootManager = ::FindWindow(NULL,TEXT("BootManager"));
////    RegisterHotKey(hBootManager,HK_NUMPAD1_ID, 0, '1');    //注册热键1，左移一个车道
////    RegisterHotKey(hBootManager,HK_NUMPAD2_ID, 0, '2');    //注册热键2，右移一个车道
////    /****20150306(end)************/
////
////    qDebug() << "All done in main, enter message loop.";
////    return g_strMyApplication->exec();
////}

MyApplication::MyApplication(int &argc,char **argv):QApplication(argc, argv)
{
    m_changeRoadSwich = 0;
}

bool MyApplication::winEventFilter(MSG *pmessage, long *result)
{    
    switch( pmessage->message)
    {
    case WM_LANENUMBER:
        qDebug()<<"Get msg WM_LANENUMBER:" << pmessage->message
               << ", pmessage->wParam:" << (int)pmessage->wParam
               << ", pmessage->lParam:" << (int)pmessage->lParam;
        if(0 == pmessage->wParam)
        {
            pmessage->wParam = 1;
            pmessage->lParam = 1;
        }
        emit MsgCamera_WM_LANENUMBER(pmessage->wParam, pmessage->lParam);
        break;
    case WM_TRAFFICSIGNAL_XYZ:
        {
            qDebug()<<"Get msg WM_TRAFFICSIGNAL_XYZ:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;
            QString strWParam, strLParam;
            float fWParam = pmessage->wParam/10.0;
            strWParam = QString("%1").arg(fWParam);

            float fLParam = pmessage->lParam/10.0;
            strLParam = QString("%1").arg(fLParam);

            emit MsgCamera_WM_TRAFFICSIGNAL_XYZ_L((const QString)strWParam);
            emit MsgCamera_WM_TRAFFICSIGNAL_XYZ_R((const QString)strLParam);
        }
        break;
    case WM_TRAFFICSIGNAL_TYPE:
        {
            qDebug()<<"Get msg WM_TRAFFICSIGNAL_TYPE:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;

            int iType = (int)pmessage->wParam;
            emit MsgCamera_WM_TRAFFICSIGNAL_TYPE(iType);
        }
        break;
    case WM_LANEDISTANCE_LD_RD:
        {
            qDebug()<<"Get msg WM_LANEDISTANCE_LD_RD:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;
            QString strWParam, strLParam;
            strWParam = QString::number(((int)pmessage->wParam));
            strLParam = QString::number(((int)pmessage->lParam));
            emit MsgCamera_WM_LANEDISTANCE_LD_RD_X((const QString)strWParam);
            emit MsgCamera_WM_LANEDISTANCE_LD_RD_Y((const QString)strLParam);
        }
        break;
    case WM_LANECHANGE_LEFT_RIGHT:
        {
            qDebug()<<"Get msg WM_LANECHANGE_LEFT_RIGHT:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;

            int iLane = 0 ;
            if(1 == pmessage->wParam)
            {
                iLane = 1;
            }

            if(1 == pmessage->lParam)
            {
                iLane = 2;
            }
            emit MsgCamera_WM_LANECHANGE_LEFT_RIGHT(iLane);
        }
        break;
    case WM_COPYDATA:
        {
            qDebug()<<"Get msg WM_COPYDATA:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;
            COPYDATASTRUCT struCopyData;

            memcpy(&struCopyData,(const void *)pmessage->lParam, sizeof(COPYDATASTRUCT));

            if(WM_NAVI_VERSION == struCopyData.dwData)
            {
                QString str;
                wchar_t ch[512] = {0};
                memcpy(ch , struCopyData.lpData , struCopyData.cbData);

                int len = WideCharToMultiByte(CP_ACP,0,ch,wcslen(ch),NULL,0,NULL,NULL);
                char *m_char=new char[len+1];
                memset(m_char,0, sizeof(m_char));

                WideCharToMultiByte(CP_ACP,0,ch,wcslen(ch),m_char,len,NULL,NULL);
                m_char[len]='\0';

                str = m_char;

                qDebug()<<"Get msg WM_NAVI_VERSION:" << struCopyData.dwData
                       << ", struCopyData.lbData:" << str
                       << ", struCopyData.cbData:" << struCopyData.cbData;

                emit MsgMobileNavigator_VERSION((const QString)str);
            }
            else if(WM_CAMERACAN_VERSION == struCopyData.dwData)
            {
                QString str;
                wchar_t ch[512] = {0};
                memcpy(ch , struCopyData.lpData , struCopyData.cbData);

                int len = WideCharToMultiByte(CP_ACP,0,ch,wcslen(ch),NULL,0,NULL,NULL);
                char *m_char=new char[len+1];
                memset(m_char,0, sizeof(m_char));

                WideCharToMultiByte(CP_ACP,0,ch,wcslen(ch),m_char,len,NULL,NULL);
                m_char[len]='\0';

                str = m_char;

                qDebug()<<"Get msg WM_CAMERACAN_VERSION:" << struCopyData.dwData
                       << ", struCopyData.lbData:" << str
                       << ", struCopyData.cbData:" << struCopyData.cbData;

                emit MsgCameraCAN_VERSION((const QString)str);
            }
            else
            {

            }
        }
        break;

    /*******20150113(begin)******/
    case WM_ENTERPARK:
        {
            qDebug()<<"Get msg WM_ENTERPARK:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;

            emit MsgMobileNavigator_EnterPark();
        }
        break;
    case WM_LEAVEPARK:
        {
            qDebug()<<"Get msg WM_LEAVEPARK:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;

            emit MsgMobileNavigator_LeavePark();
        }
        break;

    /******20150113(end)*************/

    /*****20150306(begin)********/
    case WM_NAVI_CHANGELANE_SWITCHING:
        {
            qDebug()<<"======20150306 Get msg WM_NAVI_CHANGELANE_SWITCHING:" << pmessage->message
                    <<", pmessage->wParam:" << (int)pmessage->wParam;
            m_changeRoadSwich = (int)pmessage->wParam;
        }
        break;

    case WM_HOTKEY:
        {
            qDebug()<<"======20150306 WM_HOTKEY";
            qDebug()<<"Get msg WM_HOTKEY:" << pmessage->message
                   << ", pmessage->wParam:" << (int)pmessage->wParam
                   << ", pmessage->lParam:" << (int)pmessage->lParam;
            int direction = (int)pmessage->wParam;
            qDebug()<<"======20150306 direction="<<direction;
            qDebug()<<"======20150306 m_changeRoadSwich="<<m_changeRoadSwich;
            if(1 == m_changeRoadSwich)
            {
                qDebug()<<"======20150306 open the function of changing road index by keyboard";
                emit MsgCamera_WM_CHANGEROAD(direction);  //×óóò?ü°′?üê??ˉ??±?μà?·
            }
            else if(0 == m_changeRoadSwich)
            {
                qDebug()<<"======20150306 close the function of changing road index by keyboard";
            }
            else
            {
                //do nothing
            }
        }
        break;
    /*****20150306(end)********/

    default:
        break;
    }
    return true;
    //return QApplication::winEventFilter(pmessage,result);
}

