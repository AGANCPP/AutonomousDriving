#ifndef NAVIDATA_H
#define NAVIDATA_H

/******************************************************************************
    Head files
 ******************************************************************************/
#include <QWidget>

#include "gpscontroler.h"
//#include "IHighAccuracyDataReader.h"
#include "commontools.h"

/******************************************************************************
    macro define
 ******************************************************************************/

//typedef struct MAP_POINT_{
//    double x;
//    double y;
//    void operator=(const MAP_POINT_& right)
//    {
//        x = right.x;
//        y = right.y;
//    }
//}MAP_POINT;

typedef POINT_2D MAP_POINT;

typedef struct _MAP_RECTANGLE
{
    double left;
    double bottom;
    double right;
    double top;
    _MAP_RECTANGLE()
    {
        left = 0.0;
        bottom = 0.0;
        right = 0.0;
        top = 0.0;
    }
    bool isValid()
    {
        return (((right - left) > 0.000001) &&
                ((top - bottom) > 0.000001));
    }
} MAP_RECTANGLE;

/******************************************************************************
    class define
 ******************************************************************************/
class KHADataReader;
class KHAData;
class MapData
{
public:
    MapData();
    ~MapData();

    bool loadMapData(const QString& fileName);
    MAP_RECTANGLE& calculateMapArea();
    bool isMapValid();

public:
    KHADataReader* m_pHADataReader;
    KHAData* m_pHAData;
    MAP_RECTANGLE m_mapArea;

private:
    QString m_mapFileName;
};

class HistoryTrace
{
public:
    HistoryTrace();
    ~HistoryTrace();

    bool loadHistoryTrace(const QString& fileName);
    MAP_RECTANGLE& calculateArea();
    bool isValid();

public:
    std::vector<GPS_INFO> m_historyTrace;
    MAP_RECTANGLE m_area;
};

class CurrentTrace
{
public:
    CurrentTrace();
    ~CurrentTrace();

    void updateCurrentTrace(const GPS_INFO& position);
    MAP_RECTANGLE& calculateArea();
    bool isValid();

public:
    std::vector<GPS_INFO> m_currentTrace;
    MAP_RECTANGLE m_area;
};

class NaviData
{
public:
    static NaviData& getNaviData(void);

public:
    MapData m_mapData;                        // ��ͼ����
    HistoryTrace m_historyTrace;              // ��ʷ�켣
    CurrentTrace m_currentTrace;              // ��ǰ�켣
};

// other global functions
void convertPointUnit(double x, double y, MAP_POINT& point);
double convertToDegree(unsigned long mapUnit);
void constructBoundingBox(std::vector<MAP_RECTANGLE>& areasIn, MAP_RECTANGLE& boundingBoxOut);
void WGS84toGauss(double W[], double G[]);
inline double geoCoordToMeter(double geoCoord)
{
    return (geoCoord / 35.55f);
}

inline double meterToGeoCoord(double meter)
{
    return (meter * 35.55f);
}



// kotei functions


// ���꾫��ʹ�ó�������
#define KN_GEO_DIVISION_HANDLE
#define KN_GEO_UINT                         (1024)                              // ���굥λ1/1024��
#define KN_GEO_KIWI_UNIT                    (8)                                 // Kiwi���굥λ1/8��
#define KN_GEO_REL_UINT                     ((KN_GEO_UINT)/(KN_GEO_KIWI_UNIT))  // ��Ե�λ
// γ���зֶ���
#define LATITUDE_UNIT (10) // ��10��Ϊһ��������λ
#define LATITUDE_NUM (540) // ����90��γ�ȷ�Χ������ΪLATITUDE_UNIT : (90 * 60) / LATITUDE_UNIT
#define LOGIC_LATITUDE_UNIT (614400) // ��λγ�ȶ�Ӧ���߼�����ֵ LATITUDE_UNIT * 60 * 1204

typedef struct KPoint
{
    KPoint()
    {
        X = 0;
        Y = 0;
    }

    KPoint(const KNGEOCOORD& crd)
    {
        X = crd.ulLongitude;
        Y = crd.ulLatitude;
    }

    KPoint(kn_long initX, kn_long initY)
    {
        X = initX;
        Y = initY;
    }

    kn_bool operator == (const KPoint& rhs) const
    {
        return this->X == rhs.X && this->Y == rhs.Y;
    }

    kn_bool operator != (const KPoint& rhs) const
    {
        return this->X != rhs.X || this->Y != rhs.Y;
    }

    KPoint& operator= (const KNGEOCOORD& crd)
    {
        X = crd.ulLongitude;
        Y = crd.ulLatitude;
        return *this;
    }

    kn_long X;
    kn_long Y;

} KPoint;

struct CalDisParm
{
    kn_double fDisPerPeriod;
    kn_double fLonParm;
    kn_double fLatParm;
    void clear()
    {
        fDisPerPeriod = 0.0f;
        fLonParm = 38.1096f;
        fLatParm = 33.2575f;
    }
    CalDisParm()
    {
        clear();
    }
};

// ��ṹ����
struct REPOINT
{
public:
    int x;  // ��ĺ�����
    int y;  // ���������
public:
    inline REPOINT() // Ĭ�Ϲ��캯��
    {
        x = 0;
        y = 0;
    }

    inline REPOINT(int tx, int ty) : x(tx), y(ty) // ���캯��
    {
    }

    inline REPOINT(const REPOINT& point)// ���캯��
    {
        x = point.x;
        y = point.y;
    }

    inline REPOINT& operator=(const REPOINT& point)// ���캯��
    {
        if (this == &point)
            return *this;
        x = point.x;
        y = point.y;
        return *this;
    }
};

kn_float CalAngle(const KNGEOCOORD& crd1, const KNGEOCOORD& crd2);
kn_double CalcSphericalDistanceF(const KNGEOCOORD& point1, const KNGEOCOORD& point2);

kn_double CalcSphericalDistance(const KNGEOCOORD& point1, const KNGEOCOORD& point2);
void CalLonLatParam(const KNGEOCOORD& pos, CalDisParm& disParm);
KNGEOCOORD GetNextPoint(KNGEOCOORD pos, kn_float fAngle, kn_float fMeterDis);

#define eps 1e-8
#define zero(x) (((x)>0?(x):-(x))<eps)
//���㽻��˻�(P1-P0)x(P2-P0)
double xmult(KPoint p1, KPoint p2, KPoint p0);
//�е��Ƿ����߶���,�����˵�
int dot_online_in(KPoint p, KPoint l1, KPoint l2);
//���������߶�ͬ��,�����߶��Ϸ���0
int same_side(KPoint p1, KPoint p2, KPoint l1, KPoint l2);
//����ֱ��ƽ��
int parallel(KPoint u1, KPoint u2, KPoint v1, KPoint v2);
//�����㹲��
int dots_inline(KPoint p1, KPoint p2, KPoint p3);
//�����߶��ཻ,�����˵�Ͳ����غ�
int intersect_in(KPoint u1, KPoint u2, KPoint v1, KPoint v2);
//�������߶ν���,�����߶��Ƿ��ཻ(ͬʱ����Ҫ�ж��Ƿ�ƽ��!)
KPoint intersection(KPoint u1, KPoint u2, KPoint v1, KPoint v2);
bool GetIntersectPoint(KPoint u1, KPoint u2, KPoint v1, KPoint v2, KPoint& ans);


#endif // NAVIDATA_H
