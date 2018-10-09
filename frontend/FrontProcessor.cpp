#include <fstream>
#include <iostream>
#include <algorithm>
#include <Eigen/Core>
#include <Eigen/SVD>
#include <Eigen/Cholesky>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "FrontProcessor.h"
#include "utils/Macros.h"

//KintinuousTracker
FrontProcessor::FrontProcessor(cv::Mat * Intrinsics)
 : global_time_(0),
   lastRgbImage(0),
   lastVideoImage(0)
{
    //my mark
//    intr.fx = Intrinsics->at<double>(0, 0);
//    intr.fy = Intrinsics->at<double>(1, 1);
//    intr.cx = Intrinsics->at<double>(0, 2);
//    intr.cy = Intrinsics->at<double>(1, 2);

    init_utime.assignValue(std::numeric_limits<unsigned long long>::max());
    firstRgbImage.assignValue(0);

    reset();
}

FrontProcessor::~FrontProcessor()
{

}


void FrontProcessor::reset()
{

}

void FrontProcessor::processFrame(unsigned char *rgbImage, vector<Point2f> points2d)
{
    //定位到像素坐标系
    //后续需要根据相机的rotation做判断
#ifdef WATERMARK_FUNC
    cv::Mat Image(Resolution::get().height(),Resolution::get().width(),CV_8UC3,cv::Scalar(255));
    Image.data =  rgbImage;

    cv::Mat logo = imread("icon.jpg", 1);
    cvtColor(logo, logo, CV_RGB2BGR);//CV_RGB2GRAY

    if(((int)points2d[0].x < Resolution::get().width() - logo.cols/2)
            && ((int)points2d[0].y < Resolution::get().height() - logo.rows/2)
            && (int)points2d[0].x > 0
            && (int)points2d[0].y > 0)
    {
        cv::Mat imageROI = Image(Rect((int)points2d[0].x, (int)points2d[0].y, logo.cols, logo.rows));
        cv::addWeighted(imageROI, 1.0, logo, 0.7, 0, imageROI);
        imwrite("imageROI.jpg", Image);
    }
    else{
        printf("\nERROR, OUT OF BOUND!!\n");
        return;
    }
#endif
    //传值
    lastRgbImage = rgbImage;
    printf("lastRgbImage:0x%x, strlen(lastRgbImage)=%d\n", lastRgbImage, strlen((char*)lastRgbImage));
}

void FrontProcessor::processVideoFrame(unsigned char *rgbImage)
{
    //传值
    lastVideoImage = rgbImage;
    printf("lastVideoImage:0x%x, strlen(lastVideoImage)=%d\n", lastVideoImage, strlen((char*)lastVideoImage));
}


/**
 * 功能： 1. 通过给定的欧拉角计算对应的旋转矩阵
**/
Mat FrontProcessor::eulerAnglesToRotationMatrix(Vec3f &theta)
{
    // 计算旋转矩阵的X分量
    Mat R_x = (Mat_<double>(3,3) <<
               1,       0,              0,
               0,       cos(theta[0]),   -sin(theta[0]),
               0,       sin(theta[0]),   cos(theta[0])
               );
    std::cout << R_x << std::endl;

    // 计算旋转矩阵的Y分量
    Mat R_y = (Mat_<double>(3,3) <<
               cos(theta[1]),    0,      sin(theta[1]),
               0,               1,      0,
               -sin(theta[1]),   0,      cos(theta[1])
               );
    std::cout << R_y << std::endl;

    // 计算旋转矩阵的Z分量
    Mat R_z = (Mat_<double>(3,3) <<
               cos(theta[2]),    -sin(theta[2]),      0,
               sin(theta[2]),    cos(theta[2]),       0,
               0,               0,                  1);
    std::cout << R_z << std::endl;

    // 合并
    Mat R = R_z * R_y * R_x;

    return R;
}

Mat FrontProcessor::setTMatrix()
{
    // 计算旋转矩阵的X分量
    //Mat T = (Mat_<double>(1,3) << 0.f, 0.f,1.f);
    Mat T = Mat_<double>(1,3);

    T.at<double>(0, 0) = CAMERA_POSTION_X;
    T.at<double>(0, 1) = CAMERA_POSTION_Y;
    T.at<double>(0, 2) = CAMERA_POSTION_Z;
    //std::cout << "T:" << std::endl << T << std::endl;

    return T;
}

//https://blog.csdn.net/gxsHeeN/article/details/73414015
//Opencv重投影
//projectPoints
