#ifndef COMMONTOOLS_H
#define COMMONTOOLS_H

#include <math.h>

#define PI 3.1415926

// Matrix
#define EPSINON (0.000001)
//#define EPSINON (0.00000001)

inline bool dequals(double a, double b)
{
    return (fabs(a - b) < EPSINON);
}

// time
unsigned int GetCpuTick(void);
unsigned int CalcElapsedTime(unsigned int nStart_, unsigned int nEnd_);

// Get Current work path
int GetCurrWorkPath(char* buff_, size_t size_);
// Get module location
int GetModuleLocation(char* buff_, size_t size_);
// Set current process directory
bool SetCurrProcessDir(char* dir_);

typedef struct POINT_2D_
{
    double x;
    double y;
    POINT_2D_()
    {
        x = 0.0;
        y = 0.0;
    }
    POINT_2D_(double x_, double y_)
    {
        x = x_;
        y = y_;
    }
    void operator=(const POINT_2D_& right)
    {
        x = right.x;
        y = right.y;
    }
    friend bool operator==(const POINT_2D_& left, const POINT_2D_& right)
    {
        return (dequals(left.x, right.x) && dequals(left.y, right.y));
    }
} POINT_2D;

typedef double Matrix3x3[3][3];

// Construct the 3 by 3 identity matrix
void matrix3x3SetIdentity(Matrix3x3 matIdent3x3/* in and out */);
// Premultiply matix m1 times matrix m2, store result in m2
void matrix3x3PreMultiply(Matrix3x3 m1/* in */, Matrix3x3 m2/* in and out */);
// trunslate
void translate2D(double tx, double ty, Matrix3x3 matComposite/* in and out */);
// rotate
void rotate2D(const POINT_2D& pivotPt, double theta, Matrix3x3 matComposite/* in and out */);
// scale
void scale2D(double sx, double sy, const POINT_2D& fixedPt, Matrix3x3 matComposite/* in and out */);
// Using the composite matrix, calculate transformed coordinates
void transformVerts2D(Matrix3x3 matComposite/* in */, int nVerts, POINT_2D* verts/* int and out */);


// 计算点到线段的最短距离
// 1.点积,纯量积或内积。
// Dot, scalar, or inner product.
double getPointToSegDist(const POINT_2D& p, const POINT_2D& start, const POINT_2D& end);
double getPointToSegDist(const POINT_2D& p, const POINT_2D& start, const POINT_2D& end, POINT_2D& crossPoint/*out*/);

// 求直线方程
void getLinearEquation(const POINT_2D& start, const POINT_2D& end, double& A, double& B, double& C);
// 求点P到直线距离为D的点
bool getPointAtLineWithDistance(const POINT_2D& start,
                                const POINT_2D& end,
                                const POINT_2D& p,
                                double d,
                                POINT_2D& c1,
                                POINT_2D& c2);

#endif // COMMONTOOLS_H
