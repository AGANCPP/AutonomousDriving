#ifndef MOVEMENTCONTROL_H
#define MOVEMENTCONTROL_H

#include "navidata.h"

typedef struct MATCH_TRACE_{
    MAP_POINT position;
    float     heading;        // ƫ���ǣ�0��359.99��
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

    // ���ƽ��
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
    // ��Ҫƥ��Ĺ켣
    std::vector<MATCH_TRACE> m_matchTrace;

private:
    // ��Ч���ƾ��� (�׵�λ)
    double m_validControlDistanceMetre;
    // ��Ч���ƾ��� (GPS ���굥λ)
    double m_validControlDistance;
    // ����ƫ�� (�׵�λ)
    double m_permittedOffsetMetre;
    // ����ƫ�� (GPS ���굥λ)
    double m_permittedOffset;
    // ��С��Ͼ��� (�׵�λ)
    double m_minConvergeDistanceMetre;
    // ��С��Ͼ��� (GPS ���굥λ)
    double m_minConvergeDistance;
    // ����Ͼ��� (�׵�λ)
    double m_maxConvergeDistanceMetre;
    // ����Ͼ��� (GPS ���굥λ)
    double m_maxConvergeDistance;

    // �����������
    NaviData* m_pNaviData;
    // ��ǰ�����ڹ켣��λ��
    unsigned int m_nCurMatchTraceIndex;

    // ���ƽ��
    CTL_RET m_ctlRet;
};

#endif // MOVEMENTCONTROL_H
