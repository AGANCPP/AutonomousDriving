
#include <QtCore/QtCore>
#include <QFile>
#include <qfileinfo.h>
#include <QTextStream>
#include <QDateTime>
#include <windows.h>
#include "systemlog.h"
////#include "main.h"

BOOL g_bAppLogRun = FALSE;
QString g_strCurrentDateTimeForLogFile;
QFile g_logFile;
uchar g_ucLogLevel = LOG_LEVEL_NONE;
qint64 g_iLogFileMaxSize = 0;

#define BOOTMANAGER_LOGFILE_MAXSIZE 50      //50M

QString getCurrentDateTime(uchar ucFormat)
{
    switch (ucFormat)
    {
        case EN_DATETIME_FORMAT_IN_LOG:
            return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz");
            break;
        case EN_DATETIME_FORMAT_FOR_LOGFILE:
            return QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
            break;
        default:
            return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz");
    }
}

void setCurrentLogFileName()
{
    QString lstrLogPath("log/BootManager/");
    QDir lqDir;
    if (!lqDir.exists(lstrLogPath))
        lqDir.mkpath(lstrLogPath);
    g_strCurrentDateTimeForLogFile = lstrLogPath + "bm_" + getCurrentDateTime(EN_DATETIME_FORMAT_FOR_LOGFILE);
    g_strCurrentDateTimeForLogFile += ".log";
}

int currentLogFileInit()
{
    g_ucLogLevel = 0;
    g_iLogFileMaxSize = 0;
    setCurrentLogFileName();
    //Read config file for log
    QFileInfo fConfigIniFileInfo(CONFIG_INI_FILE);
    if (fConfigIniFileInfo.exists())
    {
        QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
        g_ucLogLevel = iniSettings.value("Log/LogLevel").toInt();
        if (LOG_LEVEL_DEBUG < g_ucLogLevel)
            g_ucLogLevel = LOG_LEVEL_DEBUG;
        else if (LOG_LEVEL_NONE > g_ucLogLevel)
            g_ucLogLevel = LOG_LEVEL_NONE;
        g_iLogFileMaxSize = iniSettings.value("Log/LogSize").toUInt();
        if (0 >= g_iLogFileMaxSize)
        {
            g_iLogFileMaxSize = BOOTMANAGER_LOGFILE_MAXSIZE;   //MB
        }
        else if (BOOTMANAGER_LOGFILE_MAXSIZE < g_iLogFileMaxSize)
            g_iLogFileMaxSize = BOOTMANAGER_LOGFILE_MAXSIZE;
        g_iLogFileMaxSize = g_iLogFileMaxSize * 1024 * 1024; //Byte
    }
    return (0);
}

void customMessageHandler(QtMsgType type, const char* msg)
{
    static ulong divideNum = 0u;
    QString strCurrentDateTime = getCurrentDateTime(EN_DATETIME_FORMAT_IN_LOG);
    QString txt = strCurrentDateTime + " ";
    switch (type)
    {
        //调试信息提示
        case QtDebugMsg:
            if (g_ucLogLevel < LOG_LEVEL_DEBUG)
                return;
            txt += QString("Debug: %1").arg(msg);
            break;
        //一般的warning提示
        case QtWarningMsg:
            if (g_ucLogLevel < LOG_LEVEL_WARNING)
                return;
            txt += QString("Warning: %1").arg(msg);
            break;
        //严重错误提示
        case QtCriticalMsg:
            if (g_ucLogLevel < LOG_LEVEL_CRITICAL)
                return;
            txt += QString("Critical: %1").arg(msg);
            break;
        //致命错误提示
        case QtFatalMsg:
            if (g_ucLogLevel < LOG_LEVEL_FATAL)
                return;
            txt += QString("Fatal: %1").arg(msg);
            abort();
    }
    QFileInfo fi(g_strCurrentDateTimeForLogFile);
    if (g_iLogFileMaxSize < (fi.size()))  //size return value is byte
    {
        divideNum++;
        QString lstrLogPath("log/BootManager/");
        g_strCurrentDateTimeForLogFile = lstrLogPath + "bm_" + getCurrentDateTime(EN_DATETIME_FORMAT_FOR_LOGFILE) + "_" + QString::number(divideNum);
        g_strCurrentDateTimeForLogFile += ".log";
    }
    QFile outFile(g_strCurrentDateTimeForLogFile);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
    outFile.flush();
    outFile.close();
}


