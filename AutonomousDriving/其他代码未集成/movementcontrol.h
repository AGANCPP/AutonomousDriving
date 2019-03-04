#ifndef MOVEMENTCONTROL_H
#define MOVEMENTCONTROL_H

#include "navidata.h"

typedef struct MATCH_TRACE_{
    MAP_POINT position;
    float     heading;        // 偏航角（0～359.99）
}MATCH_TRACE;

class MovementControl
{
    friend class DrivingMapWidget;

public:
    MovementControl(NaviData* pNaviData);

    enum {
        CTL_OK,
        CTL_END,
        CTL_ERR_NO_GPS,
        CTL_ERR_NO_TRACE,
        CTL_ERR_NOT_IN_RANGE,
        CTL_ERR_COMMUNICATION_FAILED
    };
    int refresh();
    void reset();

    void reqUpdateHistoryTrace();
    void pushMatchTrace(GPS_INFO& gpsInfo);
    void clearMatchTrace(void) { m_matchTrace.clear(); }

    // 控制结果
    typedef struct CTL_RET_{
        MAP_POINT convergePoint;
        double offsetAngle;
        double wheelAngle;
        CTL_RET_()
        {
            offsetAngle = 0;
            wheelAngle = 0;
        }
    }CTL_RET;
    const CTL_RET& getCtlResult(void) { return (m_ctlRet); }

private:
    GPS_INFO* getCurrentGpsInfo(void);

public:
    // 需要匹配的轨迹
    std::vector<MATCH_TRACE> m_matchTrace;

private:
    // 有效控制距离 (米单位)
    double m_validControlDistanceMetre;
    // 有效控制距离 (GPS 坐标单位)
    double m_validControlDistance;
    // 允许偏差 (米单位)
    double m_permittedOffsetMetre;
    // 允许偏差 (GPS 坐标单位)
    double m_permittedOffset;
    // 最小汇合距离 (米单位)
    double m_minConvergeDistanceMetre;
    // 最小汇合距离 (GPS 坐标单位)
    double m_minConvergeDistance;
    // 最大汇合距离 (米单位)
    double m_maxConvergeDistanceMetre;
    // 最大汇合距离 (GPS 坐标单位)
    double m_maxConvergeDistance;

    // 导航相关数据
    NaviData* m_pNaviData;
    // 当前车所在轨迹的位置
    unsigned int m_nCurMatchTraceIndex;

    // 控制结果
    CTL_RET m_ctlRet;
};

#endif // MOVEMENTCONTROL_H
