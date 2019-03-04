// navidata.cpp

/******************************************************************************
    Head files
 ******************************************************************************/
#include "navidata.h"
#include "commondebug.h"

/******************************************************************************
    class
 ******************************************************************************/
/*
    MapData
*/
MapData::MapData()
{
    m_pHADataReader = NULL;
    m_pHAData = NULL;
    m_mapArea.left = 0;
    m_mapArea.bottom = 0;
    m_mapArea.right = 0;
    m_mapArea.top = 0;
}

MapData::~MapData()
{
    if (NULL != m_pHADataReader)
        delete(m_pHADataReader);
    if (NULL != m_pHAData)
        delete(m_pHAData);
}

bool MapData::loadMapData(const QString& fileName)
{
    KHADataReader* pHADataReader = new KHADataReader();
    if (NULL == pHADataReader)
        return (false);
    bool bRet = pHADataReader->LoadMapData((wchar_t*)(fileName.utf16()));
    if (!bRet)
    {
        delete pHADataReader;
        return (false);
    }
    KHAData* pHAData = new KHAData();
    if (NULL == pHAData)
    {
        delete pHADataReader;
        return (false);
    }
    pHADataReader->ReadAllTables(pHAData);
    if (NULL != m_pHADataReader)
        delete(m_pHADataReader);
    if (NULL != m_pHAData)
        delete(m_pHAData);
    m_pHADataReader = pHADataReader;
    m_pHAData = pHAData;
    m_mapFileName = fileName;
    calculateMapArea();
    return (true);
}

MAP_RECTANGLE& MapData::calculateMapArea()
{
    m_mapArea.left = 0;
    m_mapArea.bottom = 0;
    m_mapArea.right = 0;
    m_mapArea.top = 0;
    KHAData* pHAData = m_pHAData;
    if (NULL != pHAData)
    {
        kn_dword left = 0xFFFFFFFF;
        kn_dword bottom = 0xFFFFFFFF;
        kn_dword right = 0;
        kn_dword top = 0;
        KHI_ROAD_VECTOR_LIST::const_iterator its_roadVector = pHAData->RoadVectors().begin();
        KHI_ROAD_VECTOR_LIST::const_iterator ite_roadVector = pHAData->RoadVectors().end();
        for (; its_roadVector != ite_roadVector; ++its_roadVector)
        {
            const KHI_ROAD_VECTOR* pRoadVector = (*its_roadVector);
            if (pRoadVector->key2.dwBoxLeft < left)
                left = pRoadVector->key2.dwBoxLeft;
            if (pRoadVector->key2.dwBoxBottom < bottom)
                bottom = pRoadVector->key2.dwBoxBottom;
            if (pRoadVector->key2.dwBoxRight > right)
                right = pRoadVector->key2.dwBoxRight;
            if (pRoadVector->key2.dwBoxTop > top)
                top = pRoadVector->key2.dwBoxTop;
        }
        if (right < left)
        {
            left = 0;
            right = 500;
        }
        else if (right == left)
            right = left + 500;
        if (top < bottom)
        {
            bottom = 0;
            top = 500;
        }
        else if (top == bottom)
            top = bottom + 500;
        MAP_POINT leftBottom;
        MAP_POINT rightTop;
        convertPointUnit(left, bottom, leftBottom);
        convertPointUnit(right, top, rightTop);
        m_mapArea.left = leftBottom.x;
        m_mapArea.bottom = leftBottom.y;
        m_mapArea.right = rightTop.x;
        m_mapArea.top = rightTop.y;
    }
    qDebug("MapData::calculateMapArea[(%f, %f) (%f, %f)]\n",
           m_mapArea.left, m_mapArea.bottom, m_mapArea.right, m_mapArea.top);
    return (m_mapArea);
}

bool MapData::isMapValid()
{
    return (NULL != m_pHAData);
}


/*
    History Trace
*/
HistoryTrace::HistoryTrace()
{
    m_area.left = 0;
    m_area.bottom = 0;
    m_area.right = 0;
    m_area.top = 0;
}

HistoryTrace::~HistoryTrace()
{
}

bool HistoryTrace::loadHistoryTrace(const QString& fileName)
{
    GPSControler recvGPSData;
    QByteArray ba = fileName.toLatin1();
    m_historyTrace.clear();
    bool bRet = recvGPSData.getGpsDataFromFile(ba.data(), m_historyTrace);
    if ((false == bRet) || (0 == m_historyTrace.size()))
        return (false);
    calculateArea();
    return (true);
}

MAP_RECTANGLE& HistoryTrace::calculateArea()
{
    m_area.left = 0;
    m_area.bottom = 0;
    m_area.right = 0;
    m_area.top = 0;
    MAP_RECTANGLE box;
    box.left = 999999999.0;
    box.right = -999999999.0;
    box.top = -999999999.0;
    box.bottom = 999999999.0;
    std::vector<GPS_INFO>::iterator its = m_historyTrace.begin();
    std::vector<GPS_INFO>::iterator ite = m_historyTrace.end();
    for (; its != ite; ++its)
    {
        box.left = box.left > its->Longitude ? its->Longitude : box.left;
        box.right = box.right < its->Longitude ? its->Longitude : box.right;
        box.top = box.top < its->Lattitude ? its->Lattitude : box.top;
        box.bottom = box.bottom > its->Lattitude ? its->Lattitude : box.bottom;
    }
    if (box.right < box.left)
    {
        box.left = 0;
        box.right = 500;
    }
    else if (box.right == box.left)
        box.right = box.left + 500;
    if (box.top < box.bottom)
    {
        box.bottom = 0;
        box.top = 500;
    }
    else if (box.top == box.bottom)
        box.top = box.bottom + 500;
    MAP_POINT leftBottom;
    MAP_POINT rightTop;
    convertPointUnit(box.left, box.bottom, leftBottom);
    convertPointUnit(box.right, box.top, rightTop);
    m_area.left = leftBottom.x;
    m_area.bottom = leftBottom.y;
    m_area.right = rightTop.x;
    m_area.top = rightTop.y;
    qDebug("HistoryTrace::calculateArea[(%f, %f) (%f, %f)]\n",
           m_area.left, m_area.bottom, m_area.right, m_area.top);
    return (m_area);
}

bool HistoryTrace::isValid()
{
    return (m_historyTrace.size() > 0);
}


/*
    CurrentTrace
*/
CurrentTrace::CurrentTrace()
{
    m_area.left = 0;
    m_area.bottom = 0;
    m_area.right = 0;
    m_area.top = 0;
    // for debug
    //    GPS_INFO position1;
    //    position1.Lattitude = 30.4249530*3600*1024;
    //    position1.Longitude = 114.4315840*3600*1024;
    //    GPS_INFO position2;
    //    position2.Lattitude = 30.4249560*3600*1024;
    //    position2.Longitude = 114.4315870*3600*1024;
    //    m_currentTrace.push_back(position1);
    //    m_currentTrace.push_back(position2);
    //    calculateArea();
}

CurrentTrace::~CurrentTrace()
{
}

void CurrentTrace::updateCurrentTrace(const GPS_INFO& position)
{
    const static int max_current_trace_point_size = 1000;
    if (m_currentTrace.size() > max_current_trace_point_size)
    {
        std::vector<GPS_INFO>::iterator itBegin = m_currentTrace.begin();
        m_currentTrace.erase(itBegin, itBegin + max_current_trace_point_size / 2);
    }
    m_currentTrace.push_back(position);
}

MAP_RECTANGLE& CurrentTrace::calculateArea()
{
    m_area.left = 0;
    m_area.bottom = 0;
    m_area.right = 0;
    m_area.top = 0;
    MAP_RECTANGLE box;
    box.left = 999999999.0;
    box.right = -999999999.0;
    box.top = -999999999.0;
    box.bottom = 999999999.0;
    std::vector<GPS_INFO>::iterator its = m_currentTrace.begin();
    std::vector<GPS_INFO>::iterator ite = m_currentTrace.end();
    for (; its != ite; ++its)
    {
        box.left = box.left > its->Longitude ? its->Longitude : box.left;
        box.right = box.right < its->Longitude ? its->Longitude : box.right;
        box.top = box.top < its->Lattitude ? its->Lattitude : box.top;
        box.bottom = box.bottom > its->Lattitude ? its->Lattitude : box.bottom;
    }
    if (box.right < box.left)
    {
        box.left = 0;
        box.right = 500;
    }
    else if (box.right == box.left)
        box.right = box.left + 500;
    if (box.top < box.bottom)
    {
        box.bottom = 0;
        box.top = 500;
    }
    else if (box.top == box.bottom)
        box.top = box.bottom + 500;
    MAP_POINT leftBottom;
    MAP_POINT rightTop;
    convertPointUnit(box.left, box.bottom, leftBottom);
    convertPointUnit(box.right, box.top, rightTop);
    m_area.left = leftBottom.x;
    m_area.bottom = leftBottom.y;
    m_area.right = rightTop.x;
    m_area.top = rightTop.y;
    return (m_area);
}

bool CurrentTrace::isValid()
{
    return (m_currentTrace.size() > 0);
}

/*
    NaviData
*/
NaviData& NaviData::getNaviData(void)
{
    static NaviData s_naviData;
    return s_naviData;
}


/*
    other global functions
*/
void convertPointUnit(double x, double y, MAP_POINT& point)
{
    point.x = x;
    point.y = y;
    //    point.x = convertToDegree(x);
    //    point.y = convertToDegree(y);
}

double convertToDegree(unsigned long mapUnit)
{
    return ((double)mapUnit / 1024.0 / 3600.0);
}

void constructBoundingBox(std::vector<MAP_RECTANGLE>& areasIn, MAP_RECTANGLE& boundingBoxOut)
{
    int size = areasIn.size();
    if (size <= 0)
        return;
    boundingBoxOut = areasIn[0];
    for (int i = 1; i < areasIn.size(); ++i)
    {
        boundingBoxOut.left = boundingBoxOut.left > areasIn[i].left ? areasIn[i].left : boundingBoxOut.left;
        boundingBoxOut.right = boundingBoxOut.right < areasIn[i].right ? areasIn[i].right : boundingBoxOut.right;
        boundingBoxOut.top = boundingBoxOut.top < areasIn[i].top ? areasIn[i].top : boundingBoxOut.top;
        boundingBoxOut.bottom = boundingBoxOut.bottom > areasIn[i].bottom ? areasIn[i].bottom : boundingBoxOut.bottom;
    }
}

//////////////////////椭球参数///////////////////////////////
#define  PI  3.14159265358979
#define  ee   0.0066943799013
#define  ee2  0.00673949674227
#define  a    6378137.0000000000

void WGS84toGauss(double W[], double G[])
{
    double N, L0;
    //  double sinW0PI  = sin(  W[0]*PI/180.0);
    //  double sinW02PI = sin(2*W[0]*PI/180.0);
    //  double sinW04PI = sin(4*W[0]*PI/180.0);
    //  double sinW06PI = sin(6*W[0]*PI/180.0);
    //  double sinW08PI = sin(8*W[0]*PI/180.0);
    double cosW0PI = cos(W[0] * PI / 180.0);
#ifdef INTEL_IPP
    Ipp64f ippsinW0PI[8], ippw[8];
    ippw[0] = W[0] * PI / 180.0;
    ippw[1] = 2 * ippw[0];
    ippw[2] = 2 * ippw[1];
    ippw[3] = 3 * ippw[1];
    ippw[4] = 4 * ippw[1];
    ippsSin_64f_A50(ippw, ippsinW0PI, 5);
#else
    double ippsinW0PI[8], ippw[8];
    ippw[0] = W[0] * PI / 180.0;
    ippw[1] = 2 * ippw[0];
    ippw[2] = 2 * ippw[1];
    ippw[3] = 3 * ippw[1];
    ippw[4] = 4 * ippw[1];
    ippsinW0PI[0] = sin(ippw[0]);
    ippsinW0PI[1] = sin(ippw[1]);
    ippsinW0PI[2] = sin(ippw[2]);
    ippsinW0PI[3] = sin(ippw[3]);
    ippsinW0PI[4] = sin(ippw[4]);
#endif
    L0 = ((int)W[1] / 6 + 1) * 6 - 3;
    //  N = a/sqrt(1-ee*sinW0PI*sinW0PI);
    N = a / sqrt(1 - ee * ippsinW0PI[0] * ippsinW0PI[0]);
    double m0, m2, m4, m6, m8;
    double a0, a2, a4, a6, a8;
    m0 = a * (1 - ee);
    m2 = 3 * ee * m0 / 2;
    m4 = 5 * ee * m2 / 4;
    m6 = 7 * ee * m4 / 6;
    m8 = 9 * ee * m6 / 8;
    a0 = m0 + m2 / 2 + 3 * m4 / 8 + 5 * m6 / 16 + 35 * m8 / 128;
    a2 = m2 / 2 + m4 / 2 + 15 * m6 / 32 + 7 * m8 / 16;
    a4 = m4 / 8 + 3 * m6 / 16 + 7 * m8 / 32;
    a6 = m6 / 32 + m8 / 16;
    a8 = m8 / 128;
    double X;
    //  X=a0*W[0]*PI/180.0-a2*sinW02PI/2\
    //      +a4*sinW04PI/4\
    //      -a6*sinW06PI/6\
    //      +a8*sinW08PI/8;
    X = a0 * W[0] * PI / 180.0 - a2 * ippsinW0PI[1] / 2\
        +a4 * ippsinW0PI[2] / 4\
        -a6 * ippsinW0PI[3] / 6\
        +a8 * ippsinW0PI[4] / 8;
    double l, t, yb;
    l = (W[1] - L0) * PI / 180.0;
    t = tan(W[0] * PI / 180.0);
    yb = sqrt(ee2) * cosW0PI;
    G[0] = X + N * t * pow(cosW0PI * l, 2) / 2\
           +N * t * (5 - t * t + 9 * yb * yb\
                     +4 * pow(yb, 4)) * pow(cosW0PI * l, 4) / 24\
           +N * t * (61 - 58 * t * t + pow(t, 4)) * pow(cosW0PI * l, 6) / 720;
    G[1] = N * cosW0PI * l\
           +N / 6.0 * (1 - t * t + yb * yb) * pow((cosW0PI * l), 3)\
           +N / 120.0 * (5.0 - 18.0 * t * t\
                         +pow(t, 4) + 14 * yb * yb - 58 * yb * yb * t * t) * pow((cosW0PI * l), 5) + 500000;
    G[2] = W[2];
}

// kotei functions
kn_float s_aLonDisPerLat[540] =
{
    1.0067426f,
    1.0067362f,
    1.0067230f,
    1.0067003f,
    1.0066710f,
    1.0066286f,
    1.0065798f,
    1.0065211f,
    1.0064560f,
    1.0063812f,
    1.0062965f,
    1.0062053f,
    1.0061044f,
    1.0059969f,
    1.0058764f,
    1.0057495f,
    1.0056159f,
    1.0054727f,
    1.0053164f,
    1.0051568f,
    1.0049907f,
    1.0048084f,
    1.0046196f,
    1.0044274f,
    1.0042224f,
    1.0040075f,
    1.0037893f,
    1.0035582f,
    1.0033205f,
    1.0030731f,
    1.0028158f,
    1.0025522f,
    1.0022787f,
    1.0019988f,
    1.0017090f,
    1.0014063f,
    1.0011003f,
    1.0007877f,
    1.0004590f,
    1.0001270f,
    0.9997884f,
    0.9994369f,
    0.9990756f,
    0.9987110f,
    0.9983335f,
    0.9979494f,
    0.9975588f,
    0.9971552f,
    0.9967451f,
    0.9963285f,
    0.9958989f,
    0.9954661f,
    0.9950202f,
    0.9945679f,
    0.9941058f,
    0.9936371f,
    0.9931588f,
    0.9926739f,
    0.9921761f,
    0.9916717f,
    0.9911576f,
    0.9906402f,
    0.9901099f,
    0.9895730f,
    0.9890264f,
    0.9884701f,
    0.9879072f,
    0.9873379f,
    0.9867588f,
    0.9861699f,
    0.9855714f,
    0.9849663f,
    0.9843547f,
    0.9837302f,
    0.9831024f,
    0.9824617f,
    0.9818144f,
    0.9811574f,
    0.9804971f,
    0.9798207f,
    0.9791409f,
    0.9784515f,
    0.9777555f,
    0.9770466f,
    0.9763344f,
    0.9756125f,
    0.9748809f,
    0.9741427f,
    0.9733949f,
    0.9726374f,
    0.9718733f,
    0.9710996f,
    0.9703193f,
    0.9695293f,
    0.9687328f,
    0.9679266f,
    0.9671139f,
    0.9662915f,
    0.9654626f,
    0.9646208f,
    0.9637758f,
    0.9629209f,
    0.9620565f,
    0.9611855f,
    0.9603048f,
    0.9594176f,
    0.9585208f,
    0.9576206f,
    0.9567043f,
    0.9557848f,
    0.9548587f,
    0.9539198f,
    0.9529743f,
    0.9520192f,
    0.9510576f,
    0.9500896f,
    0.9491118f,
    0.9481276f,
    0.9471336f,
    0.9461300f,
    0.9451231f,
    0.9441032f,
    0.9430802f,
    0.9420443f,
    0.9410019f,
    0.9399529f,
    0.9388976f,
    0.9378294f,
    0.9367545f,
    0.9356735f,
    0.9345858f,
    0.9334853f,
    0.9323813f,
    0.9312680f,
    0.9301448f,
    0.9290151f,
    0.9278791f,
    0.9267334f,
    0.9255781f,
    0.9244192f,
    0.9232509f,
    0.9220729f,
    0.9208885f,
    0.9196976f,
    0.9184971f,
    0.9172900f,
    0.9160733f,
    0.9148502f,
    0.9136206f,
    0.9123813f,
    0.9111326f,
    0.9098772f,
    0.9086154f,
    0.9073471f,
    0.9060661f,
    0.9047817f,
    0.9034908f,
    0.9021873f,
    0.9008804f,
    0.8995640f,
    0.8982409f,
    0.8969085f,
    0.8955725f,
    0.8942240f,
    0.8928719f,
    0.8915105f,
    0.8901394f,
    0.8887647f,
    0.8873808f,
    0.8859904f,
    0.8845904f,
    0.8831835f,
    0.8817707f,
    0.8803481f,
    0.8789191f,
    0.8774838f,
    0.8760387f,
    0.8745872f,
    0.8731293f,
    0.8716617f,
    0.8701878f,
    0.8687074f,
    0.8672205f,
    0.8657273f,
    0.8642244f,
    0.8627151f,
    0.8611962f,
    0.8596712f,
    0.8581395f,
    0.8566012f,
    0.8550566f,
    0.8534996f,
    0.8519389f,
    0.8503718f,
    0.8487955f,
    0.8472157f,
    0.8456261f,
    0.8440274f,
    0.8424250f,
    0.8408135f,
    0.8391984f,
    0.8375708f,
    0.8359395f,
    0.8342992f,
    0.8326552f,
    0.8310021f,
    0.8293419f,
    0.8276761f,
    0.8260032f,
    0.8243212f,
    0.8226328f,
    0.8209376f,
    0.8192364f,
    0.8175257f,
    0.8158144f,
    0.8140908f,
    0.8123609f,
    0.8106272f,
    0.8088813f,
    0.8071322f,
    0.8053767f,
    0.8036116f,
    0.8018427f,
    0.8000681f,
    0.7982839f,
    0.7964932f,
    0.7946962f,
    0.7928928f,
    0.7910823f,
    0.7892662f,
    0.7874436f,
    0.7856115f,
    0.7837762f,
    0.7819345f,
    0.7800832f,
    0.7782255f,
    0.7763647f,
    0.7744943f,
    0.7726175f,
    0.7707376f,
    0.7688505f,
    0.7669546f,
    0.7650523f,
    0.7631469f,
    0.7612318f,
    0.7593105f,
    0.7573827f,
    0.7554518f,
    0.7535113f,
    0.7515644f,
    0.7496111f,
    0.7476524f,
    0.7456864f,
    0.7437173f,
    0.7417386f,
    0.7397568f,
    0.7377653f,
    0.7357708f,
    0.7337698f,
    0.7317593f,
    0.7297457f,
    0.7277257f,
    0.7256993f,
    0.7236665f,
    0.7216307f,
    0.7195853f,
    0.7175335f,
    0.7154785f,
    0.7134172f,
    0.7113496f,
    0.7092724f,
    0.7071953f,
    0.7051086f,
    0.7030155f,
    0.7009193f,
    0.6988136f,
    0.6967047f,
    0.6945894f,
    0.6924679f,
    0.6903431f,
    0.6882088f,
    0.6860715f,
    0.6839277f,
    0.6817776f,
    0.6796211f,
    0.6774616f,
    0.6752924f,
    0.6731201f,
    0.6709448f,
    0.6687598f,
    0.6665717f,
    0.6643741f,
    0.6621765f,
    0.6599694f,
    0.6577592f,
    0.6555427f,
    0.6533219f,
    0.6510926f,
    0.6488603f,
    0.6466216f,
    0.6443766f,
    0.6421284f,
    0.6398760f,
    0.6376152f,
    0.6353512f,
    0.6330810f,
    0.6308064f,
    0.6285267f,
    0.6262374f,
    0.6239482f,
    0.6216515f,
    0.6193497f,
    0.6170447f,
    0.6147322f,
    0.6124146f,
    0.6100939f,
    0.6077688f,
    0.6054354f,
    0.6031009f,
    0.6007581f,
    0.5984108f,
    0.5960585f,
    0.5937019f,
    0.5913402f,
    0.5889741f,
    0.5866029f,
    0.5842274f,
    0.5818436f,
    0.5794585f,
    0.5770704f,
    0.5746741f,
    0.5722733f,
    0.5698694f,
    0.5674573f,
    0.5650439f,
    0.5626243f,
    0.5602015f,
    0.5577738f,
    0.5553415f,
    0.5529029f,
    0.5504612f,
    0.5480164f,
    0.5455653f,
    0.5431079f,
    0.5406505f,
    0.5381836f,
    0.5357168f,
    0.5332437f,
    0.5307643f,
    0.5282817f,
    0.5257961f,
    0.5233040f,
    0.5208089f,
    0.5183108f,
    0.5158079f,
    0.5133002f,
    0.5107863f,
    0.5082693f,
    0.5057507f,
    0.5032242f,
    0.5006946f,
    0.4981635f,
    0.4956244f,
    0.4930871f,
    0.4905386f,
    0.4879918f,
    0.4854372f,
    0.4828809f,
    0.4803183f,
    0.4777544f,
    0.4751855f,
    0.4726136f,
    0.4700339f,
    0.4674557f,
    0.4648713f,
    0.4622805f,
    0.4596898f,
    0.4570928f,
    0.4544944f,
    0.4518879f,
    0.4492815f,
    0.4466721f,
    0.4440577f,
    0.4414388f,
    0.4388167f,
    0.4361916f,
    0.4335633f,
    0.4309287f,
    0.4282956f,
    0.4256548f,
    0.4230109f,
    0.4203652f,
    0.4177150f,
    0.4150631f,
    0.4124067f,
    0.4097453f,
    0.4070827f,
    0.4044151f,
    0.4017430f,
    0.3990691f,
    0.3963954f,
    0.3937140f,
    0.3910307f,
    0.3883443f,
    0.3856549f,
    0.3829623f,
    0.3802665f,
    0.3775676f,
    0.3748657f,
    0.3721606f,
    0.3694523f,
    0.3667410f,
    0.3640265f,
    0.3613101f,
    0.3585894f,
    0.3558655f,
    0.3531386f,
    0.3504096f,
    0.3476764f,
    0.3449444f,
    0.3422049f,
    0.3394667f,
    0.3367210f,
    0.3339764f,
    0.3312288f,
    0.3284769f,
    0.3257230f,
    0.3229659f,
    0.3202057f,
    0.3174456f,
    0.3146823f,
    0.3119127f,
    0.3091432f,
    0.3063705f,
    0.3035979f,
    0.3008190f,
    0.2980402f,
    0.2952582f,
    0.2924740f,
    0.2896857f,
    0.2868975f,
    0.2841071f,
    0.2813127f,
    0.2785160f,
    0.2757185f,
    0.2729155f,
    0.2701158f,
    0.2673089f,
    0.2645029f,
    0.2616937f,
    0.2588814f,
    0.2560660f,
    0.2532507f,
    0.2504321f,
    0.2476137f,
    0.2447921f,
    0.2419673f,
    0.2391426f,
    0.2363116f,
    0.2334838f,
    0.2306504f,
    0.2278196f,
    0.2249824f,
    0.2221459f,
    0.2193056f,
    0.2164661f,
    0.2136233f,
    0.2107768f,
    0.2079310f,
    0.2050852f,
    0.2022363f,
    0.1993835f,
    0.1965315f,
    0.1936763f,
    0.1908212f,
    0.1879628f,
    0.1851046f,
    0.1822437f,
    0.1793824f,
    0.1765178f,
    0.1736533f,
    0.1707862f,
    0.1679186f,
    0.1650478f,
    0.1621776f,
    0.1593069f,
    0.1564336f,
    0.1535602f,
    0.1506833f,
    0.1478068f,
    0.1449272f,
    0.1420476f,
    0.1391680f,
    0.1362852f,
    0.1334025f,
    0.1305199f,
    0.1276340f,
    0.1247482f,
    0.1218624f,
    0.1189738f,
    0.1160848f,
    0.1131959f,
    0.1103042f,
    0.1074121f,
    0.1045204f,
    0.1016252f,
    0.0987336f,
    0.0958387f,
    0.0929404f,
    0.0900455f,
    0.0871475f,
    0.0842495f,
    0.0813516f,
    0.0784504f,
    0.0755492f,
    0.0726513f,
    0.0697502f,
    0.0668459f,
    0.0639448f,
    0.0610407f,
    0.0581364f,
    0.0552323f,
    0.0523281f,
    0.0494240f,
    0.0465198f,
    0.0436125f,
    0.0407083f,
    0.0378010f,
    0.0348938f,
    0.0319865f,
    0.0290792f,
    0.0261719f,
    0.0232647f,
    0.0203574f,
    0.0174501f,
    0.0145396f,
    0.0116323f,
    0.0087251f,
    0.0058146f,
    0.0029073f
};

kn_float CalAngle(const KNGEOCOORD& crd1, const KNGEOCOORD& crd2)
{
    kn_double dAngle = 0.0;
    kn_long lLatitudeIndwx = crd1.ulLatitude / LOGIC_LATITUDE_UNIT;
    kn_double dFac = 1.0;
    //ASSERT(lLatitudeIndwx >= 0 && lLatitudeIndwx < LATITUDE_NUM);
    if (lLatitudeIndwx >= 0 && lLatitudeIndwx < LATITUDE_NUM)
        dFac = s_aLonDisPerLat[lLatitudeIndwx];
    else
        return static_cast<kn_float>(dAngle);
    if (crd2.ulLongitude - crd1.ulLongitude == 0)
    {
        if (crd1.ulLatitude <= crd2.ulLatitude)
            dAngle = 90;
        else
            dAngle = 270;
    }
    else
    {
        // ADD. hzf.2008/11/11
        // --Begin
        // 原始代码这里有两个问题要注意：
        // 1. 正负号问题，如下，如果ulLatitude2 < ulLatitude1，结果会如何？提前将ulong型转换为double型或float型（无符号型转有符号型）可处理此问题
        // 2. 精度问题，这里体现出float与double型的差异。BID_0700500001337
        // --End
        dAngle = 180.0f * atan2(((kn_double)crd2.ulLatitude - (kn_double)crd1.ulLatitude), (((kn_double)crd2.ulLongitude - (kn_double)crd1.ulLongitude) * dFac)) / PI;
        if (dAngle < 0)
            dAngle += 360.0;
    }
    return static_cast<kn_float>(dAngle);
}

kn_double CalcSphericalDistanceF(const KNGEOCOORD& point1, const KNGEOCOORD& point2)
{
    if (point1 == point2)
        return 0.0f;
    const kn_double DE2RA = 0.01745329252f;
    const kn_double FLATTENING = 1.000000f / 298.257223563f; // Earth flattening (WGS84)
    const kn_double ERAD = 6378.137f;
#ifdef KN_GEO_DIVISION_HANDLE
    const kn_double fParam = 3600.0f * KN_GEO_UINT;
#endif
#ifdef KN_GEO_SHIFT_HANDLE
    const kn_double fParam = 3600 << KN_GEO_UINT;
#endif
    kn_double lat1 = DE2RA * point1.fLatitude / fParam;
    kn_double lon1 = -DE2RA * point1.fLongitude / fParam;
    kn_double lat2 = DE2RA * point2.fLatitude / fParam;
    kn_double lon2 = -DE2RA * point2.fLongitude / fParam;
    kn_double F = (lat1 + lat2) / 2.0f;
    kn_double G = (lat1 - lat2) / 2.0f;
    kn_double L = (lon1 - lon2) / 2.0f;
    kn_double sing = sin(G);//sine(G);//
    kn_double cosl = cos(L);//cosine(L);//
    kn_double cosf = cos(F);//cosine(F);//
    kn_double sinl = sin(L);//sine(L);//
    kn_double sinf = sin(F);//sine(F);//
    kn_double cosg = cos(G);//cosine(G);//
    kn_double S = sing * sing * cosl * cosl + cosf * cosf * sinl * sinl;
    kn_double C = cosg * cosg * cosl * cosl + sinf * sinf * sinl * sinl;
    kn_double W = atan2(sqrt(S), sqrt(C));
    kn_double R = sqrt((S * C)) / W;
    kn_double H1 = (3.0f * R - 1.0f) / (2.0f * C);
    kn_double H2 = (3.0f * R + 1.0f) / (2.0f * S);
    kn_double D = 2.0f * W * ERAD;
    return (kn_double)(1.0f * 1000.0f * D * (1.0f + FLATTENING * H1 * sinf * sinf * cosg * cosg - FLATTENING * H2 * cosf * cosf * sing * sing));
}

/*
    函数功能：计算球面距离
    参数：
        point1 [in] ：球上第一个点
        point2 [in] ：球上第二个点
    返回值：距离
*/
kn_double CalcSphericalDistance(const KNGEOCOORD& point1, const KNGEOCOORD& point2)
{
    /*  const kn_double PI = 3.14159265358979323846;
        kn_double temp = 180.0 * 8.0 * 3600.0;
        kn_double m_Lon1=(kn_double)((point1.ulLongitude)*PI)/ temp;
        kn_double m_Lat1=(kn_double)((point1.ulLatitude)*PI)/ temp;
        kn_double m_Lon2=(kn_double)((point2.ulLongitude)*PI)/ temp;
        kn_double m_Lat2=(kn_double)((point2.ulLatitude)*PI)/ temp;

        kn_double a=6378137.0;
        kn_double f=1/298.257222101;
        kn_double b=a-f*a;

        kn_double cut=2;
        kn_double u1,u2,w,delta,alpha,deltam2,lamda,c,lamda1,m_Distance,m_Azim12,m_Azim21;
        u1=atan((1-f)*tan(m_Lat1));
        u2=atan((1-f)*tan(m_Lat2));
        w=m_Lon2-m_Lon1,lamda=w;

        while (cut >0.0000000001)//  (cut >0.01)
        {
            delta=acos(sin(u1)*sin(u2)+cos(u1)*cos(u2)*cos(lamda));
            alpha=asin(cos(u1)*cos(u2)*sin(lamda)/sin(delta));
            deltam2=(cos(delta)-(2*sin(u1)*sin(u2)/cos(alpha)/cos(alpha)));
            c=(f/16.0)*cos(alpha)*cos(alpha)*(4+f*(4-3*cos(alpha)*cos(alpha)));
            lamda1=w+(1-c)*f*sin(alpha)*(delta+c*sin(delta)*(deltam2+c*cos(delta)*(2*deltam2*deltam2-1)));
            cut=lamda1-lamda;
            if (cut<0)
                cut=-cut;
            lamda=lamda1;
        }
        kn_double u=cos(alpha)*cos(alpha)*(a*a-b*b)/b/b;
        kn_double A=1+(u/16384.0)*(4096 + u*(-768.0+ u*(320-175*u)));
        kn_double B=(u/1024.0)*(256.0 + u*(-128.0+ u*(74.0-47.0*u)));
        kn_double d_delta = B*sin(delta)*(deltam2 + (B/4.0)*(cos(delta)*(-1+2*deltam2*deltam2)
            -(B/6.0)*deltam2*(-3+4*sin(delta)*sin(delta))*(-3+4*deltam2*deltam2)));

        m_Distance=b*A*(delta - d_delta);
        m_Azim12 =atan((cos(u2)*sin(lamda))/(cos(u1)*sin(u2)-sin(u1)*cos(u2)*cos(lamda)));
        m_Azim21 = atan((cos(u1)*sin(lamda))/(-cos(u2)*sin(u1)+sin(u2)*cos(u1)*cos(lamda)));
        return (kn_int)( 1.0 * m_Distance);
    */
    if (point1 == point2)
        return 0.0f;
    const kn_double DE2RA = 0.01745329252f;
    const kn_double FLATTENING = 1.000000f / 298.257223563f; // Earth flattening (WGS84)
    const kn_double ERAD = 6378.137f;
#ifdef KN_GEO_DIVISION_HANDLE
    const kn_double fParam = 3600.0f * KN_GEO_UINT;
#endif
#ifdef KN_GEO_SHIFT_HANDLE
    const kn_double fParam = 3600 << KN_GEO_UINT;
#endif
    kn_double lat1 = DE2RA * point1.fLatitude / fParam;
    kn_double lon1 = -DE2RA * point1.fLongitude / fParam;
    kn_double lat2 = DE2RA * point2.fLatitude / fParam;
    kn_double lon2 = -DE2RA * point2.fLongitude / fParam;
    kn_double F = (lat1 + lat2) / 2.0f;
    kn_double G = (lat1 - lat2) / 2.0f;
    kn_double L = (lon1 - lon2) / 2.0f;
    kn_double sing = sin(G);//sine(G);//
    kn_double cosl = cos(L);//cosine(L);//
    kn_double cosf = cos(F);//cosine(F);//
    kn_double sinl = sin(L);//sine(L);//
    kn_double sinf = sin(F);//sine(F);//
    kn_double cosg = cos(G);//cosine(G);//
    kn_double S = sing * sing * cosl * cosl + cosf * cosf * sinl * sinl;
    kn_double C = cosg * cosg * cosl * cosl + sinf * sinf * sinl * sinl;
    kn_double W = atan2(sqrt(S), sqrt(C));
    kn_double R = sqrt((S * C)) / W;
    kn_double H1 = (3.0f * R - 1.0f) / (2.0f * C);
    kn_double H2 = (3.0f * R + 1.0f) / (2.0f * S);
    kn_double D = 2.0f * W * ERAD;
    return (kn_double)(1.0f * 1000.0f * D * (1.0f + FLATTENING * H1 * sinf * sinf * cosg * cosg - FLATTENING * H2 * cosf * cosf * sing * sing));
}

/**
    功能：计算当前坐标点附近的经纬度换算为米的单位

    @param pos  经纬度坐标
    @param calDisParm  经纬度坐标与米的换算单位
    返回值：
*/
void CalLonLatParam(const KNGEOCOORD& pos, CalDisParm& disParm)
{
    KNGEOCOORD Start, End;
    Start.fLongitude = pos.fLongitude;
    Start.fLatitude = pos.fLatitude;
    End.fLongitude = Start.fLongitude + (kn_double)102400.0f;
    End.fLatitude = Start.fLatitude;
    Start.trans();
    End.trans();
    kn_double fArcDis = CalcSphericalDistance(Start, End);
    disParm.fLonParm = (kn_double)102400.0f / fArcDis;
    End.fLongitude = Start.fLongitude;
    End.fLatitude = Start.fLatitude + (kn_double)102400.0f;
    End.trans();
    fArcDis = CalcSphericalDistance(Start, End);
    disParm.fLatParm = (kn_double)102400.0f / fArcDis;
}

KNGEOCOORD GetNextPoint(KNGEOCOORD pos, kn_float fAngle, kn_float fMeterDis)
{
    KNGEOCOORD nextPos;
    if (fMeterDis < 0.0001f)
    {
        nextPos = pos;
        return nextPos;
    }
    CalDisParm disParm;                       // 米转经纬度的参数
    kn_double fRadian = 0.0174532922222222f;  // PI / 180.0f
    CalLonLatParam(pos, disParm);
    kn_double fCos = cos(fRadian * (kn_double)fAngle);
    kn_double fSin = sin(fRadian * (kn_double)fAngle);
    nextPos.fLongitude = pos.fLongitude + fCos * (kn_double)fMeterDis * disParm.fLonParm;
    nextPos.fLatitude  = pos.fLatitude + fSin * (kn_double)fMeterDis * disParm.fLatParm;
    nextPos.ulLongitude = static_cast<kn_ulong>(nextPos.fLongitude);
    nextPos.ulLatitude = static_cast<kn_ulong>(nextPos.fLatitude);
    //nextPos.ulLongitude = static_cast<kn_ulong>(pos.ulLongitude + fCos * (kn_double)fMeterDis * m_DisParm.fLonParm);
    //nextPos.ulLatitude = static_cast<kn_ulong>(pos.ulLatitude + fSin * (kn_double)fMeterDis * m_DisParm.fLatParm);
    return nextPos;
}

//#########################################################################################################################################//
// 线段的交点

//计算交叉乘积(P1-P0)x(P2-P0)
double xmult(KPoint p1, KPoint p2, KPoint p0)
{
    return (double)(p1.X - p0.X) * (double)(p2.Y - p0.Y) - (double)(p2.X - p0.X) * (double)(p1.Y - p0.Y);
}

//判点是否在线段上,包括端点
int dot_online_in(KPoint p, KPoint l1, KPoint l2)
{
    return zero(xmult(p, l1, l2)) && (double)(l1.X - p.X) * (double)(l2.X - p.X) < eps && (double)(l1.Y - p.Y) * (double)(l2.Y - p.Y) < eps;
}

//判两点在线段同侧,点在线段上返回0
int same_side(KPoint p1, KPoint p2, KPoint l1, KPoint l2)
{
    return xmult(l1, p1, l2) * xmult(l1, p2, l2) > eps;
}

//判两直线平行
int parallel(KPoint u1, KPoint u2, KPoint v1, KPoint v2)
{
    return zero((double)(u1.X - u2.X) * (double)(v1.Y - v2.Y) - (double)(v1.X - v2.X) * (double)(u1.Y - u2.Y));
}

//判三点共线
int dots_inline(KPoint p1, KPoint p2, KPoint p3)
{
    return zero(xmult(p1, p2, p3));
}

//判两线段相交,包括端点和部分重合
int intersect_in(KPoint u1, KPoint u2, KPoint v1, KPoint v2)
{
    if (!dots_inline(u1, u2, v1) || !dots_inline(u1, u2, v2))
        return (!same_side(u1, u2, v1, v2) && !same_side(v1, v2, u1, u2));
    return (dot_online_in(u1, v1, v2) || dot_online_in(u2, v1, v2) || dot_online_in(v1, u1, u2) || dot_online_in(v2, u1, u2));
}

//计算两线段交点,请判线段是否相交(同时还是要判断是否平行!)
KPoint intersection(KPoint u1, KPoint u2, KPoint v1, KPoint v2)
{
    KPoint ret = u1;
    double t = ((double)(u1.X - v1.X) * (double)(v1.Y - v2.Y) - (double)(u1.Y - v1.Y) * (double)(v1.X - v2.X)) / ((double)(u1.X - u2.X) * (double)(v1.Y - v2.Y) - (double)(u1.Y - u2.Y) * (double)(v1.X - v2.X));
    ret.X += (double)(u2.X - u1.X) * t;
    ret.Y += (double)(u2.Y - u1.Y) * t;
    return ret;
}

bool GetIntersectPoint(KPoint u1, KPoint u2, KPoint v1, KPoint v2, KPoint& ans)
{
    if (parallel(u1, u2, v1, v2) || !intersect_in(u1, u2, v1, v2))
        return false;
    else
    {
        ans = intersection(u1, u2, v1, v2);
        return true;
    }
    return false;
}


//#########################################################################################################################################//
