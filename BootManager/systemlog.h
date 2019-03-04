#ifndef SYSTEMLOG_H
#define SYSTEMLOG_H

typedef enum _EN_DATETIME_FORMAT_TYPE
{
    EN_DATETIME_FORMAT_IN_LOG = 0,
    EN_DATETIME_FORMAT_FOR_LOGFILE
} EN_DATETIME_FORMAT_TYPE;

//0:none;1:fatal;2:critical;3:warning;4:debug;
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_CRITICAL 2
#define LOG_LEVEL_FATAL 1
#define LOG_LEVEL_NONE 0

#define CONFIG_INI_FILE "./config.ini"
#define CONFIG_DEF_SCR_WIDTH 1280
#define CONFIG_DEF_SCR_HEIGHT 1024
#define CONFIG_DEF_MAIN_WIDTH 480
#define CONFIG_DEF_MAIN_HEIGHT 424
#define CONFIG_DEF_NAVI_WIDTH 800
#define CONFIG_DEF_NAVI_HEIGHT 600
#define CONFIG_DEF_LANE_WIDTH 800
#define CONFIG_DEF_LANE_HEIGHT 424
#define CONFIG_DEF_SIGN_WIDTH 480
#define CONFIG_DEF_SIGN_HEIGHT 320
#define CONFIG_DEF_ROAD_WIDTH 480
#define CONFIG_DEF_ROAD_HEIGHT 280

enum emTrafficSignalID
{
    TS_NULL = 0,
    TS_TRAFFIC_GUIDEBOARD,                      // 路牌
    TS_REGU_RATE_LIMIT,                         // 车辆限速
    TS_UTILITY_EMERGENCY_CALL,                  // 紧急电话
};




void setCurrentLogFileName();
QString getCurrentDateTime(uchar ucFormat);
int currentLogFileInit();
void customMessageHandler(QtMsgType type, const char* msg);

#endif // SYSTEMLOG_H
