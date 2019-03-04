#include "movementcontrol.h"
#include "IpcAccess.h"
#include "commondebug.h"

MovementControl::MovementControl(NaviData *pNaviData)
{
    m_pNaviData = pNaviData;

    // ��Ч���ƾ��� (�׵�λ)
    m_validControlDistanceMetre = 10.0;
    // ��Ч���ƾ��� (GPS ���굥λ)
    m_validControlDistance = meterToGeoCoord(m_validControlDistanceMetre);
    // ����ƫ�� (�׵�λ)
    m_permittedOffsetMetre = 0.5;
    // ����ƫ�� (GPS ���굥λ)
    m_permittedOffset = meterToGeoCoord(m_permittedOffsetMetre);
    // ��С��Ͼ��� (�׵�λ)
    m_minConvergeDistanceMetre = 8.0;
    // ��С��Ͼ��� (GPS ���굥λ)
    m_minConvergeDistance = meterToGeoCoord(m_minConvergeDistanceMetre);
    // ����Ͼ��� (�׵�λ)
    m_maxConvergeDistanceMetre = 16.0;
    // ����Ͼ��� (GPS ���굥λ)
    m_maxConvergeDistance = meterToGeoCoord(m_maxConvergeDistanceMetre);

    m_nCurMatchTraceIndex = 0;
}

void MovementControl::reqUpdateHistoryTrace()
{
    if (NULL != m_pNaviData) {
        clearMatchTrace();
        int size = m_pNaviData->m_historyTrace.m_historyTrace.size();
        HistoryTrace* pHistoryTrace = &(m_pNaviData->m_historyTrace);
        for (int i = 0; i < size; ++i) {
            pushMatchTrace(pHistoryTrace->m_historyTrace[i]);
        }
    }
}

void MovementControl::pushMatchTrace(GPS_INFO& gpsInfo)
{
    MATCH_TRACE matchTace;
    convertPointUnit(gpsInfo.Longitude, gpsInfo.Lattitude, matchTace.position);
    matchTace.heading = gpsInfo.Heading;
    m_matchTrace.push_back(matchTace);
}

// ���³�����̬
// �Ƕȣ� ��תΪ������תΪ����
int MovementControl::refresh()
{
    // ��ȡ��ǰλ����Ϣ
    GPS_INFO* pCurrGpsInfo = getCurrentGpsInfo();
    if (NULL == pCurrGpsInfo) {
        return (CTL_ERR_NO_GPS);
    }

    // �ж��Ƿ��й켣����
    unsigned int nTraceSize = m_matchTrace.size();
    if (0 == nTraceSize) {
        return (CTL_ERR_NO_TRACE);
    }

    if ((nTraceSize) < 10) {
        return (CTL_END);
    }

    // ���㵱ǰλ��
    MAP_POINT currentPoint;
    convertPointUnit(pCurrGpsInfo->Longitude, pCurrGpsInfo->Lattitude, currentPoint);

    // ���㵱ǰλ����켣��ƫ��
    double tmpDistance = 0.0;
    double nearestDistance = 999999999.0;
    MAP_POINT tmpPoint;
    MAP_POINT nearestPoint;
    unsigned int nStartIndex = (m_nCurMatchTraceIndex > 100) ? (m_nCurMatchTraceIndex - 100) : m_nCurMatchTraceIndex;
    for (unsigned int i = nStartIndex; i < nTraceSize; ++i) {
        // ��������Ĺ켣��
        if (i > 0) {
            tmpDistance = getPointToSegDist(currentPoint,
                                            m_matchTrace[i - 1].position,
                                            m_matchTrace[i].position,
                                            tmpPoint);
        } else {
            tmpDistance = sqrt((currentPoint.x - m_matchTrace[i].position.x) * (currentPoint.x - m_matchTrace[i].position.x) +
                               (currentPoint.y - m_matchTrace[i].position.y) * (currentPoint.y - m_matchTrace[i].position.y));
            tmpPoint = m_matchTrace[i].position;
        }
        if (tmpDistance < nearestDistance) {
            nearestDistance = tmpDistance;
            nearestPoint = tmpPoint;
            m_nCurMatchTraceIndex = i;
        }
    }

    // �ж�ƫ������Ƿ������Ч���ƾ���
    if (nearestDistance > m_validControlDistance) {
        return (CTL_ERR_NOT_IN_RANGE);
    }

    // �ж��Ƿ��ѵ��յ�
    if ((nTraceSize - m_nCurMatchTraceIndex) < 5) {
        return (CTL_END);
    }

    double offsetAngle = 0.0;

    // �ж�ƫ������Ƿ��������ƫ��
    if (nearestDistance < m_permittedOffset) {
        // ������ƫ�Χ��
        // ���㵱ǰ���ƫ������켣�ϵ�ƫ���ǵ�ƫ��
        offsetAngle = -m_matchTrace[m_nCurMatchTraceIndex].heading + pCurrGpsInfo->Heading;
        m_ctlRet.convergePoint = m_matchTrace[m_nCurMatchTraceIndex].position;
    } else {
        // ������ƫ�Χ�⣬��Ҫ����������̬
        // �ڹ켣�У����뵱ǰ�������С�ĵ㿪ʼ��ǰ�������ҵ�������ʵĻ���㣬���������ת��
        bool bMatchRet = false;
        unsigned int nMatchIndex;
        for (unsigned int i = m_nCurMatchTraceIndex; i < nTraceSize; ++i) {
            tmpDistance = sqrt((currentPoint.x - m_matchTrace[i].position.x) * (currentPoint.x - m_matchTrace[i].position.x) +
                               (currentPoint.y - m_matchTrace[i].position.y) * (currentPoint.y - m_matchTrace[i].position.y));
            if (tmpDistance < m_minConvergeDistance) {
                continue;
            } else if (tmpDistance < m_maxConvergeDistance) {
                bMatchRet = true;
                nMatchIndex = i;
                tmpPoint = m_matchTrace[i].position;
                COM_DEBUG(6, ("ADC: MovementControl::refresh, 1 converge point(%f, %f) distance(%f)", tmpPoint.x, tmpPoint.y, tmpDistance));
                break;
            } else {
                if (i >= 1) {
#if 0
                    bMatchRet = true;
                    nMatchIndex = i;
                    tmpPoint.x = (m_matchTrace[i].position.x + m_matchTrace[i - 1].position.x) / 2.0;
                    tmpPoint.y = (m_matchTrace[i].position.y + m_matchTrace[i - 1].position.y) / 2.0;
                    COM_DEBUG(6, ("ADC: MovementControl::refresh, 2 converge point(%f, %f) distance(%f)", tmpPoint.x, tmpPoint.y, tmpDistance));
#else
                    double d = (m_minConvergeDistance + m_maxConvergeDistance) / 2.0;
                    MAP_POINT c1;
                    MAP_POINT c2;
                    bool bRet = getPointAtLineWithDistance(m_matchTrace[i - 1].position,
                                                           m_matchTrace[i].position,
                                                           currentPoint,
                                                           d,
                                                           c1,
                                                           c2);
                    if (false == bRet) {
                        COM_ERR("[ERR]ADC: 1 getPointAtLineWithDistance error, start(%f, %f) end(%f, %f), p(%f, %f), d(%f)",
                                m_matchTrace[i - 1].position.x, m_matchTrace[i - 1].position.y,
                                m_matchTrace[i].position.x, m_matchTrace[i].position.y,
                                currentPoint.x, currentPoint.y,
                                d);
                        bMatchRet = true;
                        nMatchIndex = i;
                        tmpPoint.x = (m_matchTrace[i].position.x + m_matchTrace[i - 1].position.x) / 2.0;
                        tmpPoint.y = (m_matchTrace[i].position.y + m_matchTrace[i - 1].position.y) / 2.0;
                        break;
                    } else {
                        double minx = 0;
                        double miny = 0;
                        double maxx = 0;
                        double maxy = 0;
                        if (m_matchTrace[i - 1].position.x < m_matchTrace[i].position.x) {
                            minx = m_matchTrace[i - 1].position.x;
                            maxx = m_matchTrace[i].position.x;
                        } else {
                            minx = m_matchTrace[i].position.x;
                            maxx = m_matchTrace[i - 1].position.x;
                        }
                        if (m_matchTrace[i - 1].position.y < m_matchTrace[i].position.y) {
                            miny = m_matchTrace[i - 1].position.y;
                            maxy = m_matchTrace[i].position.y;
                        } else {
                            miny = m_matchTrace[i].position.y;
                            maxy = m_matchTrace[i - 1].position.y;
                        }
                        if ((c1.x >= minx) && (c1.y >= miny) && (c1.x <= maxx) && (c1.y <= maxy)) {
                            bMatchRet = true;
                            nMatchIndex = i;
                            tmpPoint = c1;
                            COM_DEBUG(6, ("ADC: MovementControl::refresh, 2 converge point(%f, %f) distance(%f)", tmpPoint.x, tmpPoint.y, tmpDistance));
                            break;
                        } else if ((c2.x >= minx) && (c2.y >= miny) && (c2.x <= maxx) && (c2.y <= maxy)) {
                            bMatchRet = true;
                            nMatchIndex = i;
                            tmpPoint = c2;
                            COM_DEBUG(6, ("ADC: MovementControl::refresh, 3 converge point(%f, %f) distance(%f)", tmpPoint.x, tmpPoint.y, tmpDistance));
                            break;
                        } else {
                            COM_ERR("[ERR]ADC: 2 getPointAtLineWithDistance error, start(%f, %f) end(%f, %f), p(%f, %f), d(%f)",
                                    m_matchTrace[i - 1].position.x, m_matchTrace[i - 1].position.y,
                                    m_matchTrace[i].position.x, m_matchTrace[i].position.y,
                                    currentPoint.x, currentPoint.y,
                                    d);
                            bMatchRet = true;
                            nMatchIndex = i;
                            tmpPoint.x = (m_matchTrace[i].position.x + m_matchTrace[i - 1].position.x) / 2.0;
                            tmpPoint.y = (m_matchTrace[i].position.y + m_matchTrace[i - 1].position.y) / 2.0;
                            break;
                        }
                    }
#endif
                } else {
                    bMatchRet = true;
                    nMatchIndex = i;
                    tmpPoint = m_matchTrace[i].position;
                    COM_DEBUG(6, ("ADC: MovementControl::refresh, 4 converge point(%f, %f) distance(%f)", tmpPoint.x, tmpPoint.y, tmpDistance));
                }
                break;
            }
        }

        if (!bMatchRet) {
            nMatchIndex = nTraceSize - 1;
            tmpPoint = m_matchTrace[nTraceSize - 1].position;
        }

        // �������ת��
        double v1_x = tmpPoint.x - currentPoint.x;
        double v1_y = tmpPoint.y - currentPoint.y;
        tmpDistance = sqrt((v1_x) * (v1_x) + (v1_y) * (v1_y));
        double convergeAngle = asin((tmpPoint.y - currentPoint.y) / tmpDistance)* 180.0 / PI;
        if ((v1_x >= 0) && (v1_y >= 0)) {
            // ��һ����
        } else if ((v1_x < 0) && (v1_y >= 0)) {
            // �ڶ�����
            convergeAngle = 180 - convergeAngle;
        } else if ((v1_x < 0) && (v1_y < 0)) {
            // ��������
            convergeAngle = 180 - convergeAngle;
        } else {
            // ��������
            convergeAngle = 360 + convergeAngle;
        }

        convergeAngle = 90 - convergeAngle;
        if (convergeAngle < 0) {
            convergeAngle += 360;
        }

        COM_DEBUG(6, ("ADC: MovementControl::refresh, converge angle(%f) distance(%f)", convergeAngle, tmpDistance));

        // ���㵱ǰ���ƫ���������ת�ǵ�ƫ��
        offsetAngle = -convergeAngle + pCurrGpsInfo->Heading;

        m_ctlRet.convergePoint = tmpPoint;
    }

    if (offsetAngle > 180.0) {
        offsetAngle -= 360.0;
    } else if (offsetAngle < -180.0) {
        offsetAngle += 360.0;
    }

    int wheelAngle = 0.0;
    if (offsetAngle > 80) {
        wheelAngle = 35;
    } else if (offsetAngle < -80) {
        wheelAngle = -35;
    } else {
        wheelAngle = offsetAngle * 35 / 80;
    }

    wheelAngle *= 15;

    COM_DEBUG(6, ("ADC: MovementControl::refresh, current(%f) offset(%f) wheel(%d)", pCurrGpsInfo->Heading, offsetAngle, wheelAngle));

    m_ctlRet.offsetAngle = offsetAngle;
    m_ctlRet.wheelAngle = wheelAngle;

    // ����ת������
    int ret = ReqVehTurnWheel(wheelAngle, 5);
    if (IPC_OK != ret) {
        return (CTL_ERR_COMMUNICATION_FAILED);
    }

    return (CTL_OK);
}

void MovementControl::reset()
{
    m_nCurMatchTraceIndex = 0;
}

GPS_INFO* MovementControl::getCurrentGpsInfo(void)
{
    if (NULL != m_pNaviData) {
        int size = m_pNaviData->m_currentTrace.m_currentTrace.size();
        if (size > 0) {
            return (&(m_pNaviData->m_currentTrace.m_currentTrace[size-1]));
        }
    }

    return (NULL);
}
