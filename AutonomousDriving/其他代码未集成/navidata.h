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
    MapData m_mapData;                        // 地图数据
    HistoryTrace m_historyTrace;              // 历史轨迹
    CurrentTrace m_currentTrace;              // 当前轨迹
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


// 坐标精度使用除法处理
#define KN_GEO_DIVISION_HANDLE
#define KN_GEO_UINT                         (1024)                              // 坐标单位1/1024秒
#define KN_GEO_KIWI_UNIT                    (8)                                 // Kiwi坐标单位1/8秒
#define KN_GEO_REL_UINT                     ((KN_GEO_UINT)/(KN_GEO_KIWI_UNIT))  // 相对单位
// 纬度切分定义
#define LATITUDE_UNIT (10) // 以10分为一个基本单位
#define LATITUDE_NUM (540) // 考察90度纬度范围，换算为LATITUDE_UNIT : (90 * 60) / LATITUDE_UNIT
#define LOGIC_LATITUDE_UNIT (614400) // 单位纬度对应的逻辑坐标值 LATITUDE_UNIT * 60 * 1204

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

// 点结构定义
struct REPOINT
{
public:
    int x;  // 点的横坐标
    int y;  // 点的纵坐标
public:
    inline REPOINT() // 默认构造函数
    {
        x = 0;
        y = 0;
    }

    inline REPOINT(int tx, int ty) : x(tx), y(ty) // 构造函数
    {
    }

    inline REPOINT(const REPOINT& point)// 构造函数
    {
        x = point.x;
        y = point.y;
    }

    inline REPOINT& operator=(const REPOINT& point)// 构造函数
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
//计算交叉乘积(P1-P0)x(P2-P0)
double xmult(KPoint p1, KPoint p2, KPoint p0);
//判点是否在线段上,包括端点
int dot_online_in(KPoint p, KPoint l1, KPoint l2);
//判两点在线段同侧,点在线段上返回0
int same_side(KPoint p1, KPoint p2, KPoint l1, KPoint l2);
//判两直线平行
int parallel(KPoint u1, KPoint u2, KPoint v1, KPoint v2);
//判三点共线
int dots_inline(KPoint p1, KPoint p2, KPoint p3);
//判两线段相交,包括端点和部分重合
int intersect_in(KPoint u1, KPoint u2, KPoint v1, KPoint v2);
//计算两线段交点,请判线段是否相交(同时还是要判断是否平行!)
KPoint intersection(KPoint u1, KPoint u2, KPoint v1, KPoint v2);
bool GetIntersectPoint(KPoint u1, KPoint u2, KPoint v1, KPoint v2, KPoint& ans);


#endif // NAVIDATA_H
