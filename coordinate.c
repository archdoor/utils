#include <stdio.h>
#include <math.h>
#include <string.h>
#include "coordinate.h"

// 地球半径(单位：米)
static const int g_earth_radius = 6371004;
// 圆周率
static const double PI = 3.1415926535897931;

// WGS84度分秒转化成度
double wgs84_to_angle(double coordinate)
{
    double dd = floor(coordinate);
    double mm = floor((coordinate - dd)*100);
    double ss = ((coordinate - dd)*100 - mm)*100;

    return (dd + mm/60 + ss/3600);
}

// GPS度分转化成度
double gps_to_angle(double coordinate)
{
    double dd = floor(coordinate/100);
    double mm = coordinate - (dd*100);

    return (dd + mm/60);
}


// 度转化成GSP度分
double angle_to_gps(double coordinate)
{
    double dd = floor(coordinate);
    double mm = (coordinate - dd) * 60;

    return (dd*100 + mm);
}

// IEEE754转化成双精度值
double ieee754_to_double(double data754)
{
    // 整数部分
    float *integer_field = (float *)&data754;
    // 小数部分
    float *decimal_field = integer_field + 1;

    return (*integer_field + *decimal_field);
}

// 双精度值转化成IEEE754
double double_to_ieee754(double data)
{
    double ieee754 = 0;

    // 整数部分
    float integer_field = floor(data);
    // 小数部分
    float decimal_field = data - integer_field;

    float *p = (float *)&ieee754;
    memcpy(p, &integer_field, 4);
    memcpy(p + 1, &decimal_field, 4);

    return ieee754;
}


// AB两点间弧线距离
double point_to_point(EarthCoordinate coor1, EarthCoordinate coor2)
{
    double aj = coor1.lon;
    double aw = coor1.lat;
    double bj = coor2.lon;
    double bw = coor2.lat;

    // 角度转化成弧度
    aj =(aj * PI)/180;  
    aw =(aw * PI)/180;  
    bj =(bj * PI)/180;  
    bw =(bw * PI)/180;  

    // AB点经度差
    double dlon = fabs(aj - bj);
    // AB点纬度差
    double dlat = fabs(aw - bw);

    // A点所在纬度圈半径
    double a_radius = g_earth_radius * cos(aw);
    // B点所在纬度圈半径
    double b_radius = g_earth_radius * cos(bw);

    // AB两点间的A点纬线直线距离(梯形上边长)
    double dis_aw = a_radius * sin(dlon/2) * 2;
    // AB两点间的B点纬线直线距离(梯形下边长)
    double dis_bw = b_radius * sin(dlon/2) * 2;

    // AB两点间经线直线距离(梯形腰长)
    double dis_abj = g_earth_radius * sin(dlat/2) * 2;

    // AB两点的直线距离(已知等腰梯形的四边长度，求对角线长度)
    double dis_ab = sqrt(pow(dis_abj, 2) + (dis_aw * dis_bw));

    // AB和地心所成夹角弧度
    double radius_aob = 2 * asin(dis_ab / ( 2 * g_earth_radius));

    // AB之间的弧线距离
    double dis_radius_ab = g_earth_radius * radius_aob;

    return dis_radius_ab;
}

// 点C到线AB的弧线地面距离
double point_radius_to_line(EarthCoordinate coor_start, EarthCoordinate coor_end, EarthCoordinate coor_active)
{
    // 求三角形ABC的三边长
    double ab = point_to_point(coor_start, coor_end);
    double ac = point_to_point(coor_start, coor_active);
    double bc = point_to_point(coor_end, coor_active);

    // 已知四面体六条边长，求OABC四面体体积(固定公式)
    double a = pow(ab, 2);
    double b = pow(ac, 2);
    double c = pow(bc, 2);
    double r = pow(g_earth_radius, 2);
    double volume = (sqrt(r*a*(r+r+b+c-r-a)+r*b*(r+r+a+c-r-b)+r*c*(r+r+a+b-r-c)-r*r*c-r*r*a-r*r*b-a*b*c))/12; 

    // 海伦公式四面体底面OAB面积
    double half_girth = (ab + g_earth_radius + g_earth_radius)/2;
    double area = sqrt(half_girth * (half_girth - ab) * (half_girth - g_earth_radius) * (half_girth - g_earth_radius));

    // 根据体积求四面体高度(点C到平面OAB垂线长度)
    double h = (3 * volume)/area;

    // 求直线OC与平面OAB的夹角
    double cod = asin(h/g_earth_radius);

    // 计算地面实际弧线距离
    double radius_dis = g_earth_radius * cod;
     
    return radius_dis;
}

// 点C到线AB的垂直距离
double point_vertical_to_line(EarthCoordinate coor_start, EarthCoordinate coor_end, EarthCoordinate coor_active)
{
    // 求三角形ABC的三边长
    double ab = point_to_point(coor_start, coor_end);
    double ac = point_to_point(coor_start, coor_active);
    double bc = point_to_point(coor_end, coor_active);

    // 三角形半周长
    double half_girth = (ab + ac + bc)/2;
    // 海伦公式求三角形ABC面积
    double area = sqrt(half_girth * (half_girth - ab) * (half_girth - ac) * (half_girth - bc));
    // 求C到直线AB的距离
    double height = (2 * area)/ab;

    return height;
}

// 判断点C是否在AB经度或纬度之间
bool is_between(EarthCoordinate coor_start, EarthCoordinate coor_end, EarthCoordinate coor_active)
{
    double between_j = (coor_start.lon - coor_active.lon) * (coor_end.lon - coor_active.lon);
    double between_w = (coor_start.lat - coor_active.lat) * (coor_end.lat - coor_active.lat);
    if ( (between_j > 0) && (between_w > 0) )
    {
        return false;
    }
    return true;
}

// 经纬度坐标转换成80西安坐标
void angle_to_80(EarthCoordinate earth, PlaneCoordinate *plane)
{
    double j = earth.lon;
    double w = earth.lat;

    // 长轴
    double a = 6378140.0;
    // 短轴
    double b = 6356755.288157528;
    // 扁率
    // double f = (a - b) / a;
    // 第一偏心率平方
    double e1 = 1 - (b / a) * (b / a);;
    // 第二偏心率平方
    double e2 = (a / b) * (a / b) - 1;
    // 角度弧度转换参数
    double dblPI = PI / 180.0;

    // 角度转弧度
    double L = j * dblPI;
    double B = w * dblPI;

    double m0 = a * (1 - e1);
    double m2 = (3.0/2.0) * e1 * m0;
    double m4 = 5 * e1 * m2;
    double m6 = (7.0/6.0) * e1 * m4;
    double m8 = (9.0/8.0) * e1 * m6;

    double a0 = m0 + m2/2 + (3.0/8.0)*m4 + (5.0/16.0)*m6 + (35.0/128.0)*m8;
    double a2 = m2/2 + m4/2 + (15.0/32.0)*m6 + (7.0/16.0)*m8;
    double a4 = m4/8 + (3.0/16.0)*m6 + (7.0/32.0)*m8;
    double a6 = m6/32 + m8/16;
    // double a8 = m8/128;

    double X = a0*B - sin(B)*cos(B)*((a2-a4+a6)+(2*a4-(16.0/3.0)*a6)*pow(sin(B),2)+(16.0/3.0)*a6*pow(sin(B),4));

    double N = a/sqrt(1 - e1*pow(sin(B),2));
    double t = tan(B);
    double q = e2 * pow(cos(B), 2);
    // double p = (180.0/PI) * 3600.0;
    double p = 1;
    double L0 = ((int)(j/3)*3.0) * dblPI;

    double I = L - L0;

    double x_axis = X+(N/(2*pow(p,2)))*sin(B)*cos(B)*pow(I,2) + (N/(24.0*pow(p,4)))*sin(B)*pow(cos(B),3)*(5-pow(t,2)+9*q+4.0*pow(q,2))*pow(I,4) + (N/(720.0*pow(p,6)))*sin(B)*pow(cos(B),5)*(61-58.0*pow(t,2)+pow(t,4))*pow(I,6);
    double y_axis = (N/p)*cos(B)*I + (N/(6*pow(p,3)))*pow(cos(B),3)*(1-pow(t,2)+q)*pow(I,3) + (N/(120*pow(p,5)))*pow(cos(B),5)*(5-18*pow(t,2)+pow(t,4)+14*q-58*q*pow(t,2))*pow(I,5);

    // 高斯投影正算的Y轴是水平轴，所以要转换成正规二维坐标轴
    plane->x = y_axis + 500000;
    plane->y = x_axis;
}






#ifdef UTILS_COORDINATE_DEBUG

int main()
{
    // 经纬度转换成平面坐标
    double coordinate[] = {
        117.475933889, 39.156774639,
        117.476270278, 39.156619889,
        117.476188611, 39.156511667,
        117.475852222, 39.156666417,
    };

    EarthCoordinate earth = {0};
    PlaneCoordinate coor = {0};
    for ( unsigned int i = 0; i < sizeof(coordinate)/sizeof(double); )
    {
        earth.lon = coordinate[i];
        earth.lat = coordinate[i + 1];
        angle_to_80(earth, &coor);
        printf("%-15.6f\t%-15.6f\n", coor.x, coor.y);

        i += 2;
    }

    return 0;
}

#endif

