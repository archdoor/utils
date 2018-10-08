#ifndef __COORDINATE_H__
#define __COORDINATE_H__

#include <stdbool.h>

// 平面坐标
typedef struct
{
    double x;
    double y;
}PlaneCoordinate;

// 地球坐标(GPS坐标)
typedef struct
{
    double lon; // 经度
    double lat; // 纬度
}EarthCoordinate;


// WGS84度分秒转化成度
double wgs84_to_angle(double coordinate);


// GPS度分转化成度
double gps_to_angle(double coordinate);
// 度转化成GPS度分
double angle_to_gps(double coordinate);

// IEEE754转化成双精度值
double ieee754_to_double(double data754);
// 双精度值转化成IEEE754
double double_to_ieee754(double data);


// AB两点间弧线距离
double point_to_point(EarthCoordinate coor1, EarthCoordinate coor2);

// 点C到线AB的弧线地面距离
double point_radius_to_line(EarthCoordinate coor_start, EarthCoordinate coor_end, EarthCoordinate coor_active);

// 点C到线AB的垂直距离
double point_vertical_to_line(EarthCoordinate coor_start, EarthCoordinate coor_end, EarthCoordinate coor_active);

// 判断点C是否在AB经度或纬度之间
bool is_between(EarthCoordinate coor_start, EarthCoordinate coor_end, EarthCoordinate coor_active);

// 经纬度坐标转换成80西安坐标
void angle_to_80(EarthCoordinate earth, PlaneCoordinate *plane);

#endif


