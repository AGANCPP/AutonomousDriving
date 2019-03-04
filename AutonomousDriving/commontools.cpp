//
#include "commontools.h"
#include <Windows.h>
#include <math.h>

// time
unsigned int GetCpuTick(void)
{
    return (unsigned int)GetTickCount();
}

unsigned int CalcElapsedTime(unsigned int nStart_, unsigned int nEnd_)
{
    if (nEnd_ >= nStart_)
        return (nEnd_ - nStart_);
    else
        return (0xFFFFFFFF - nStart_ + nEnd_ + 1);
}

// work path
int GetCurrWorkPath(char* buff_, size_t size_)
{
    return (GetCurrentDirectoryA(size_, buff_));
}

int GetModuleLocation(char* buff_, size_t size_)
{
    //char full_name[MAX_PATH] = { 0 };
    int size = 0;
    char* pCut = NULL;
    size = GetModuleFileNameA(NULL, buff_, size_);
    //printf("full module name = %s\n", full_name);
    if (pCut = strrchr(buff_, '\\'))
    {
        *(pCut) = 0;
        size = strlen(buff_);
    }
    else
        size = 0;
    return (size);
}

bool SetCurrProcessDir(char* dir_)
{
    return (SetCurrentDirectoryA(dir_));
}

/*
    Matrix
*/

// Construct the 3 by 3 identity matrix
void matrix3x3SetIdentity(Matrix3x3 matIdent3x3)
{
    int row, col;
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
            matIdent3x3[row][col] = (row == col);
    }
}

// Premultiply matix m1 times matrix m2, store result in m2
void matrix3x3PreMultiply(Matrix3x3 m1, Matrix3x3 m2)
{
    int row, col;
    Matrix3x3 matTemp;
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
        {
            matTemp[row][col] = m1[row][0] * m2[0][col] + m1[row][1] *
                                m2[1][col] + m1[row][2] * m2[2][col];
        }
    }
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
            m2[row][col] = matTemp[row][col];
    }
}

// trunslate
void translate2D(double tx, double ty, Matrix3x3 matComposite)
{
    Matrix3x3 matTransl;
    // Initialize translation matrix to identity
    matrix3x3SetIdentity(matTransl);
    matTransl[0][2] = tx;
    matTransl[1][2] = ty;
    // Concatenate matTransl with the composite matrix
    matrix3x3PreMultiply(matTransl, matComposite);
}

// rotate
void rotate2D(const POINT_2D& pivotPt, double theta, Matrix3x3 matComposite)
{
    Matrix3x3 matRot;
    // Initialize rotation matrix to identity
    matrix3x3SetIdentity(matRot);
    matRot[0][0] = cos(theta);
    matRot[0][1] = -sin(theta);
    matRot[0][2] = pivotPt.x * (1.0 - cos(theta)) + pivotPt.y * sin(theta);
    matRot[1][0] = sin(theta);
    matRot[1][1] = cos(theta);
    matRot[1][2] = pivotPt.y * (1.0 - cos(theta)) - pivotPt.x * sin(theta);
    // Concatenate matRot with the composite matrix
    matrix3x3PreMultiply(matRot, matComposite);
}

// scale
void scale2D(double sx, double sy, const POINT_2D& fixedPt, Matrix3x3 matComposite)
{
    Matrix3x3 matScale;
    // Initialize scaling matrix to identity
    matrix3x3SetIdentity(matScale);
    matScale[0][0] = sx;
    matScale[0][2] = (1 - sx) * fixedPt.x;
    matScale[1][1] = sy;
    matScale[1][2] = (1 - sy) * fixedPt.y;
    // Concatenate matScale with the composite matrix
    matrix3x3PreMultiply(matScale, matComposite);
}

// Using the composite matrix, calculate transformed coordinates
void transformVerts2D(Matrix3x3 matComposite, int nVerts, POINT_2D* verts)
{
    int k;
    double temp;
    for (k = 0; k < nVerts; k++)
    {
        temp = matComposite[0][0] * verts[k].x + matComposite[0][1] *
               verts[k].y + matComposite[0][2];
        verts[k].y = matComposite[1][0] * verts[k].x + matComposite[1][1] *
                     verts[k].y + matComposite[1][2];
        verts[k].x = temp;
    }
}

// 计算点到线段的最短距离
// 1.点积,纯量积或内积。
// Dot, scalar, or inner product.
double getPointToSegDist(const POINT_2D& p, const POINT_2D& start, const POINT_2D& end)
{
    double end_start_x = end.x - start.x;
    double end_start_y = end.y - start.y;
    double p_start_x = p.x - start.x;
    double p_start_y = p.y - start.y;
    double inner_product_start_p_start_end =
        (end_start_x) * (p_start_x) + (end_start_y) * (p_start_y);
    if (inner_product_start_p_start_end <= 0)
    {
        // |start p|
        //
        //        p
        //
        //    ----c---start-------end
        //
        return sqrt((p_start_x) * (p_start_x) + (p_start_y) * (p_start_y));
    }
    double square_distance_start_end =
        (end_start_x) * (end_start_x) + (end_start_y) * (end_start_y);
    if (inner_product_start_p_start_end >= square_distance_start_end)
    {
        // |end p|
        //
        //                        p
        //
        //    start-------end-----c
        //
        return sqrt((p.x - end.x) * (p.x - end.x) + (p.y - end.y) * (p.y - end.y));
    }
    double r = inner_product_start_p_start_end / square_distance_start_end;
    double cx = start.x + (end_start_x) * r;
    double cy = start.y + (end_start_y) * r;
    // |c p|
    //
    //            p
    //
    //    start---c---end
    //
    return sqrt((p.x - cx) * (p.x - cx) + (p.y - cy) * (p.y - cy));
}

double getPointToSegDist(const POINT_2D& p, const POINT_2D& start, const POINT_2D& end, POINT_2D& crossPoint/*out*/)
{
    double end_start_x = end.x - start.x;
    double end_start_y = end.y - start.y;
    double p_start_x = p.x - start.x;
    double p_start_y = p.y - start.y;
    double inner_product_start_p_start_end =
        (end_start_x) * (p_start_x) + (end_start_y) * (p_start_y);
    if (inner_product_start_p_start_end <= 0)
    {
        // |start p|
        //
        //        p
        //
        //    ----c---start-------end
        //
        crossPoint = start;
        return sqrt((p_start_x) * (p_start_x) + (p_start_y) * (p_start_y));
    }
    double square_distance_start_end =
        (end_start_x) * (end_start_x) + (end_start_y) * (end_start_y);
    if (inner_product_start_p_start_end >= square_distance_start_end)
    {
        // |end p|
        //
        //                        p
        //
        //    start-------end-----c
        //
        crossPoint = end;
        return sqrt((p.x - end.x) * (p.x - end.x) + (p.y - end.y) * (p.y - end.y));
    }
    double r = inner_product_start_p_start_end / square_distance_start_end;
    double cx = start.x + (end_start_x) * r;
    double cy = start.y + (end_start_y) * r;
    // |c p|
    //
    //            p
    //
    //    start---c---end
    //
    crossPoint.x = cx;
    crossPoint.y = cy;
    return sqrt((p.x - cx) * (p.x - cx) + (p.y - cy) * (p.y - cy));
}

// 求直线方程
void getLinearEquation(const POINT_2D& start, const POINT_2D& end, double& A, double& B, double& C)
{
    A = end.y - start.y;
    B = start.x - end.x;
    C = end.x * start.y - start.x * end.y;
}

// 求点P到直线距离为D的点
bool getPointAtLineWithDistance(const POINT_2D& start,
                                const POINT_2D& end,
                                const POINT_2D& p,
                                double d,
                                POINT_2D& c1,
                                POINT_2D& c2)
{
    if ((start == end) || (d < EPSINON))
        return (false);
    // 求直线方程
    double A = 0, B = 0, C = 0;
    getLinearEquation(start, end, A, B, C);
    // 求解p.x
    double a = (A * A) / (B * B) + 1.0;
    double b = 2.0 * (-p.x + (A * C) / (B * B) + (p.y * A) / B);
    double c = (C * C) / (B * B) + (2 * p.y * C) / B + p.x * p.x + p.y * p.y - d * d;
    double b2_4ac = b * b - 4.0 * a * c;
    if (b2_4ac < 0)
        return (false);
    double sqrt_b2_4ac = sqrt(b2_4ac);
    double x1 = (-b + sqrt_b2_4ac) / (2.0 * a);
    double y1 = (-A * x1 - C) / B;
    double x2 = (-b - sqrt_b2_4ac) / (2.0 * a);
    double y2 = (-A * x2 - C) / B;
    c1.x = x1;
    c1.y = y1;
    c2.x = x2;
    c2.y = y2;
    return (true);
}
