#ifndef SLAM_H
#define SLAM_H

#include <Eigen/Dense> //http://stackoverflow.com/questions/9876209/using-eigen-library-with-opencv-2-3-1

#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>

// opencv 特征检测模块
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <string>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <iostream>

#include <pthread.h>

//#include <opencv/cv.h>
//#include <opencv/cxcore.h>
//#include <opencv/highgui.h>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

#include <opencv2/opencv.hpp>

#include <string>

#include <iostream>
#include <stdio.h>

//Clog exp_log;

// PCL head files
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>

//#include "list.h"

#include <pcl/segmentation/supervoxel_clustering.h>
#include <vtkPolyLine.h>

#include <pcl/common/transforms.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/passthrough.h>
#include <pcl/registration/icp.h>

#include <g2o/types/slam3d/types_slam3d.h>
#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/factory.h>
#include <g2o/core/optimization_algorithm_factory.h>
#include <g2o/core/optimization_algorithm_gauss_newton.h>
#include <g2o/solvers/csparse/linear_solver_csparse.h>
#include <g2o/core/robust_kernel.h>
#include <g2o/core/robust_kernel_factory.h>
#include <g2o/core/optimization_algorithm_levenberg.h>



#include "ArRobot.h"

#define USEg2o 0
#define SAVEg2o 0

#define LOOP_DETECT (0&&USEg2o)

#define SAVEGLOBAL 1
#define USE_VIEWER 1

#include <sys/time.h>
#include <unistd.h>

using namespace std;
using std::vector;
using std::string;


//typedef pcl::PointXYZ Point;
typedef pcl::PointXYZRGB PointT;
typedef pcl::PointCloud<PointT> PointCloudT;
typedef pcl::PointNormal PointNT;
typedef pcl::PointCloud<PointNT> PointNCloudT;
typedef pcl::PointXYZL PointLT;
typedef pcl::PointCloud<PointLT> PointLCloudT;

typedef pcl::PointXYZRGBA PointTA;
typedef pcl::PointCloud<PointTA> PointCloudTA;


typedef vector<PointT> PTARRAY;

#define PCLVisualization 0
#define ImgBlurring 0
#define ImgStore 0
#define PcdStore 0

#define USE_IR 1

#define CreatePCL 1
#define ImgConcate 0
#define CloudConcate ImgConcate
#define UseCluster 1

//#define EPS 1e-6

#define SHOWSIFTIV 0 // intermediate variable

#define MIN_INLIERS 5
#define MAX_NORM 0.3
#define MAX_NORM_LP 5.0
#define KEYFRAME_THRESHOLD 0.0

#define assert_e(a) {if (!(a)) exit(-1);}


struct CAMERA_INTRINSIC_PARAMETERS {
    double cx, cy, fx, fy, scale;
};

class ImageList;
class ImageDB;

struct QBY {
    libfreenect2::Frame *rgb, *depth;
    libfreenect2::Freenect2Device::IrCameraParams *ir_params;
    libfreenect2::Freenect2Device::ColorCameraParams *color_params;
    ImageList *current; ImageDB *imgdb;
};


void* slam_event(void* args);
extern ImageDB imgdb;

cv::Point3f point2dTo3d (cv::Point3f& point, CAMERA_INTRINSIC_PARAMETERS& camera);
PointCloudT::Ptr image2PointCloud( cv::Mat& rgb, cv::Mat& depth, CAMERA_INTRINSIC_PARAMETERS& camera );

#define USE_SUPERVOXEL_CLUSTERS 1

bool CompareVector(const PointT &pt1, const PointT &pt2);
PTARRAY ConvexHull(PointCloudT &cluster);

#define EdgeDebug (0 && ShowEdgeResult)
#define ShowEdgeResult 0

extern float voxel_resolution;
extern float seed_resolution;

extern float posdist;
extern float negdist;
extern int min_clusters_contains;


#if ShowEdgeResult
void drawEdge(vector<PTARRAY> &final_edges, const pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud, bool hold,
                         boost::shared_ptr<pcl::visualization::PCLVisualizer> & viewer);
#else
void drawEdge(vector<PTARRAY> &final_edges, const pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud, bool hold);
#endif


Eigen::Isometry3d cvMat2Eigen( cv::Mat& rvec, cv::Mat& tvec );
double normofTransform(cv::Mat rvec, cv::Mat tvec);

class ImageList {
  public:
    ImageList(int id_) {
        origin = false; id = id_;
#if USE_IR
        img = new cv::Mat(424, 512, CV_8UC1);
#else
        //img = new cv::Mat(424, 512, CV_8UC3);//CV_8UC4);
#endif
        depth = new cv::Mat(424, 512, CV_16UC1);//CV_32FC1);
        cloud = pcl::PointCloud<pcl::PointXYZRGB>::Ptr(new pcl::PointCloud<pcl::PointXYZRGB>);


        person_point = pcl::PointXYZRGB(0,0,0);
    }
    ~ImageList() {
        delete img;
        delete depth;
    }
    void clear() {}
    void storePcl() {
        char pcd_name[20];
        sprintf(pcd_name, "cloud_%d.pcd", id);
        pcl::io::savePCDFile(pcd_name, *cloud);
    }
    void storeImg() {
        char img_name[20];
        sprintf(img_name, "Img/raw_img_%d.jpg", id);
        imwrite(img_name, *img);
        sprintf(img_name, "Depth/depth_%d.jpg", id);
        imwrite(img_name, *depth);
    }
    void showPcl() {
        char viewer_name[20];
        sprintf(viewer_name, "CloudViewer_%d", id);
        pcl::visualization::CloudViewer viewer(viewer_name);
        viewer.showCloud(cloud, viewer_name);
        while (!viewer.wasStopped());
    }
    void blurImg() {
#if USE_IR
        cv::Mat* img_blur = new cv::Mat(424, 512, CV_8UC1);
#else
        cv::Mat* img_blur = new cv::Mat(424, 512, CV_8UC3);//CV_8UC4);
#endif
        cv::Mat* depth_blur = new cv::Mat(424, 512, CV_16UC1);//CV_32FC1);

        medianBlur(*img_blur, *img, 5);
        medianBlur(*depth_blur, *depth, 5);
        delete img; delete depth;
        img = img_blur; depth = depth_blur;
    }

    struct timeval time;
    cv::Mat *img;
    cv::Mat *depth;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud;
    Eigen::Isometry3d T;

    bool origin;
    PointT pos;
    int id;
    Eigen::Isometry3d globalT; // of no use if origin == true

    // following function used by SIFT
    void showKeyPoints(const vector<cv::KeyPoint>& keypoints1) {
        cv::Mat img_show;
        char img_name[20];
        cv::drawKeypoints(*img, keypoints1, img_show, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        sprintf(img_name, "keypoints_%d", id);
        cv::imshow(img_name, img_show);
        sprintf(img_name+strlen(img_name), ".png");
        cv::imwrite(img_name, img_show);
        cv::waitKey(0);
    }
    void showKeyPoints(const vector<cv::KeyPoint>& keypoints1, const ImageList* other, const vector<cv::KeyPoint>& keypoints2, const vector<cv::DMatch>& matches, string prefix = "") {
        assert_e(other->id > id);
        cv::Mat img_matches;
        char img_name[20];
        cv::drawMatches(*img, keypoints1, *(other->img), keypoints2, matches, img_matches);
        sprintf(img_name, "%smatches_%d_%d", prefix.c_str(), id, other->id);
        cv::imshow(img_name, img_matches);
        sprintf(img_name+strlen(img_name), ".png");
        cv::imwrite(img_name, img_matches);
        cv::waitKey(0);
    }

    vector<PTARRAY> edge;
    struct timeval collect_time;
    double collect_x, collect_y;
    pcl::PointXYZRGB person_point;
};

// function for new thread
void* edge_event(void* args);
void* qby_event(void* args);
void* concate_event(void* args);

extern g2o::RobustKernel* robustKernel;

class ImageDB {
  public:
    ImageDB(int len = 120) {
        list_len = len;
        imglists = (ImageList**)malloc(list_len * sizeof(ImageList*));
        for (int i = 0; i < list_len-1; i++) {
            imglists[i] = new ImageList(i);
        }
        current_id = -1;
        edge_thread_in_use = concate_thread_in_use = qby_thread_in_use = false;
        edge_thread_item = concate_thread_item = NULL;

        base = PointCloudT::Ptr( new PointCloudT() );
        local_viewer = global_viewer = NULL; robot = NULL;

    }
    void setViewer(boost::shared_ptr<pcl::visualization::PCLVisualizer> local,
                   boost::shared_ptr<pcl::visualization::PCLVisualizer> global) {
        local_viewer = local; global_viewer = global;
    }
    void unsetViewer() { local_viewer = global_viewer = NULL; }
    ImageList* add() {
        if (++current_id >= list_len) return NULL;
        return imglists[current_id];
    }
    void del() { current_id--; }

    int current_id, list_len;
    struct ImageList** imglists;

    CAMERA_INTRINSIC_PARAMETERS C;
    boost::shared_ptr<pcl::visualization::PCLVisualizer> local_viewer, global_viewer; 

     // ********************************************
    ArRobot *robot;
    void setRobot(ArRobot *r) {
        robot = r;
    }

    // ********************************************
    // threads for EdgeDetect
    pthread_t edge_thread, qby_thread;
    bool edge_thread_in_use, qby_thread_in_use;
    ImageList* edge_thread_item;

    ImageList* getEdge() { return edge_thread_item; }
    void stage_edge(int id, libfreenect2::Frame *rgb, libfreenect2::Frame *depth,
                    libfreenect2::Freenect2Device::IrCameraParams *ir_params,
                    libfreenect2::Freenect2Device::ColorCameraParams *rgb_params) {
        int Rc;
        ImageList *current = imglists[current_id];
        assert_e(id == current_id);

        if (!edge_thread_in_use && !qby_thread_in_use) {
            edge_thread_in_use = qby_thread_in_use = true;
            Rc = pthread_create(&edge_thread, NULL, edge_event, (void*)this);
//edge_thread_in_use = false;
            printf("create new thread %d\n", Rc);

            QBY *qby_param = new QBY;
            uint32_t *rgb_data = (uint32_t*)malloc(sizeof(uint32_t)*1080*1920); 
            float *depth_data = (float*)malloc(sizeof(float)*424*512);
            memcpy(rgb_data, rgb->data, sizeof(uint32_t)*1080*1920);
            memcpy(depth_data, depth->data, sizeof(float)*424*512);
            qby_param->rgb = new libfreenect2::Frame(rgb->width, rgb->height, (size_t)sizeof(uint32_t), (unsigned char*)rgb_data);
            qby_param->depth = new libfreenect2::Frame(depth->width, depth->height, (size_t)sizeof(float), (unsigned char *)depth_data);
            qby_param->ir_params = ir_params; qby_param->color_params = rgb_params;
            qby_param->current = imglists[id]; qby_param->imgdb = this;
            Rc = pthread_create(&qby_thread, NULL, qby_event, (void*)qby_param);
            printf("create qby thread %d\n", Rc);
        }
    }

    // ********************************************
    // threads for ImgConcatenate
    PointCloudT::Ptr base;
    pthread_t concate_thread;
    bool concate_thread_in_use;
    ImageList* concate_thread_item;

    //ImageList* getEdge() { return edge_thread_item; }
    void stage_concate(int id) {
        int Rc;
        ImageList *current = imglists[current_id];
        assert_e(id == current_id);

        if (current_id) {
            printf("Start new thread for concatenate .");
            while (!concate_thread_in_use) {
                concate_thread_in_use = true;
                Rc = pthread_create(&concate_thread, NULL, concate_event, (void*)this);
                if (Rc) printf(".");
                usleep(50);
            }
            printf("\nStart end.\n");
        }
        else {
            *base += *(current->cloud);
        }
    }

    // ********************************************
    // Transform Matrix
        // PnP 结果
    struct RESULT_OF_PNP {
        cv::Mat rvec, tvec;
        int inliers;
    };
    RESULT_OF_PNP estimateMotion(int src_id, int target_id) {
        assert_e(src_id && target_id < src_id && src_id == current_id);
        ImageList *current = imglists[src_id], *last = imglists[target_id];
        const cv::Mat *rgb1 = current->img,
                      *rgb2 = last->img;
        const cv::Mat *depth1 = current->depth,
                      *depth2 = last->depth;

        // 声明特征提取器与描述子提取器
        cv::Ptr<cv::FeatureDetector> _detector;
        cv::Ptr<cv::DescriptorExtractor> _descriptor;

        // 构建提取器，默认两者都为sift
        // 构建sift, surf之前要初始化nonfree模块
        cv::initModule_nonfree();
        _detector = cv::FeatureDetector::create( "GridSIFT" );
        _descriptor = cv::DescriptorExtractor::create( "SIFT" );

        vector< cv::KeyPoint > kp1, kp2; //关键点
        _detector->detect( *rgb1, kp1 );  //提取关键点
        _detector->detect( *rgb2, kp2 );
        printf("Key points of two images: %lu, %lu\n", kp1.size(), kp2.size());
                
        // 可视化， 显示关键点
        if (SHOWSIFTIV) current->showKeyPoints(kp1);
               
        // 计算描述子
        cv::Mat desp1, desp2;
        _descriptor->compute( *rgb1, kp1, desp1 );
        _descriptor->compute( *rgb2, kp2, desp2 );

        // 匹配描述子
        vector< cv::DMatch > matches; 
        cv::FlannBasedMatcher matcher;
        matcher.match( desp1, desp2, matches );
        printf("Find total %lu matches\n", matches.size());

        // 可视化：显示匹配的特征
        if (SHOWSIFTIV) current->showKeyPoints(kp1, last, kp2, matches);

        // 筛选匹配，把距离太大的去掉
        // 这里使用的准则是去掉大于四倍最小距离的匹配
        vector< cv::DMatch > goodMatches;
        double minDis = 9999;
        for ( size_t i=0; i<matches.size(); i++ ) {
            if ( matches[i].distance < minDis ) minDis = matches[i].distance;
        }

        for ( size_t i=0; i<matches.size(); i++ ) {
            if (matches[i].distance < 4.0*minDis) goodMatches.push_back( matches[i] );
        }

        // 显示 good matches
        if (SHOWSIFTIV) current->showKeyPoints(kp1, last, kp2, goodMatches);

        // 计算图像间的运动关系
        // 关键函数：cv::solvePnPRansac()
        // 为调用此函数准备必要的参数
                
        // 第一个帧的三维点
        vector<cv::Point3f> pts_obj;
        // 第二个帧的图像点
        vector< cv::Point2f > pts_img;

        for (size_t i=0; i<goodMatches.size(); i++) {
            // query 是第一个, train 是第二个
            cv::Point2f p = kp1[goodMatches[i].queryIdx].pt;
            cv::Point2f p2 = kp2[goodMatches[i].trainIdx].pt;
            // 获取d是要小心！x是向右的，y是向下的，所以y才是行，x是列！
            ushort d = depth1->at<ushort>(int(p.y),int(p.x));
            ushort d2 = depth2->at<ushort>(int(p2.y),int(p2.x));
            //cout << "( y: " << p.y << " x: " << p.x << "depth: " <<  d <<  "depth2 : " << d2<< ')' << endl; 
            //ushort d = depth1.ptr<ushort>( int(p.y) )[ int(p.x) ];
            int tmpcount = 0;
            if (d == 0) {
                for (int j = 0; j < 5; j++) {
                    if (int(p.x) - j < 0) break;
                    if (depth1->at<ushort>(int(p.y),int(p.x) - j) != 0) {
                        d = depth1->at<ushort>(int(p.y),int(p.x) - j);
                        tmpcount = j;
                        break;
                    }
                }
            }
            if (d == 0) continue;
            //cout << "newpoint: ( y: " << p.y << " x: " << p.x << "depth: " <<  d <<  "depth2 : " << d2<< ')' << endl; 
            pts_img.push_back( cv::Point2f( kp2[goodMatches[i].trainIdx].pt ) );

            // 将(u,v,d)转成(x,y,z)
            cv::Point3f pt ( p.x - tmpcount, p.y, d );
            cv::Point3f pd = point2dTo3d( pt, C );
            pts_obj.push_back( pd );
        }

        double camera_matrix_data[3][3] = {
            {C.fx, 0, C.cx},
            {0, C.fy, C.cy},
            {0, 0, 1}
        };

        printf("SIZE:%lu %lu\n", pts_img.size(), pts_obj.size());
        // 构建相机矩阵
        cv::Mat cameraMatrix( 3, 3, CV_64F, camera_matrix_data );
        cv::Mat rvec, tvec, inliers;
        // 求解pnp
        if (pts_img.size() >= MIN_INLIERS && pts_obj.size() >= MIN_INLIERS) {
            cv::solvePnPRansac( pts_obj, pts_img, cameraMatrix, cv::Mat(), rvec, tvec, false, 100, 1.0, 100, inliers );
            //cout<<"inliers: "<<inliers.rows<<endl;
            //cout<<"R="<<rvec<<endl;
            //cout<<"t="<<tvec<<endl;
        }
        else inliers.rows = 0;
        printf("inliers: %d\n", inliers.rows);

        RESULT_OF_PNP result;
        result.rvec = rvec;
        result.tvec = tvec;
        result.inliers = inliers.rows;

        return result;
    }

#define ABANDON_IF_NOT_MATCH 0
    bool stage_sift(int id) {
        assert_e(id == current_id);
        ImageList *current = imglists[id];
        if (current->id == 0) return true;

        double norm;
        RESULT_OF_PNP result = estimateMotion(id, id-1);

        if (result.inliers >= MIN_INLIERS) {
            norm = normofTransform(result.rvec, result.tvec);
            printf("norm=%lf\n", norm);
        }

        if (result.inliers < MIN_INLIERS || norm > MAX_NORM || norm < KEYFRAME_THRESHOLD) {
#if ABANDON_IF_NOT_MATCH
            return false;
#else
            current->T = Eigen::Isometry3d::Identity();
            return true;
#endif
        }
        else {
            // 画出inliers匹配
            /*if (SHOWSIFTIV) { // TODO: recover this
                vector< cv::DMatch > matchesShow;
                for (size_t i=0; i<inliers.rows; i++) {
                    matchesShow.push_back( goodMatches[inliers.ptr<int>(i)[0]] );    
                }
                current->showKeyPoints(kp1, last, kp2, matchesShow, "inliers");
            }*/

            current->T = cvMat2Eigen(result.rvec, result.tvec);
            return true;
        }
    }

    // TODO: set SparseOptimizer as a parameter of this class
    //bool checkFrameMatch(int src_id, int target_id, g2o::SparseOptimizer& opti);
#define NEARBY_LOOPS 2 // 5
    bool checkFrameMatch(int src_id, int target_id, g2o::SparseOptimizer& opti) {
        assert_e( src_id > target_id && src_id == current_id );

        RESULT_OF_PNP result = estimateMotion(src_id, target_id);
        if (result.inliers < MIN_INLIERS) return false;

        // 计算运动范围是否太大
        double norm = normofTransform(result.rvec, result.tvec);
        if (norm > MAX_NORM_LP || norm <= KEYFRAME_THRESHOLD) return false;

        // 边部分
        g2o::EdgeSE3* edge = new g2o::EdgeSE3();
        // 连接此边的两个顶点id
        edge->vertices() [0] = opti.vertex(src_id);
        edge->vertices() [1] = opti.vertex( target_id);
        edge->setRobustKernel( robustKernel );
        // 信息矩阵
        Eigen::Matrix<double, 6, 6> information = Eigen::Matrix< double, 6,6 >::Identity();
        // 信息矩阵是协方差矩阵的逆，表示我们对边的精度的预先估计
        // 因为pose为6D的，信息矩阵是6*6的阵，假设位置和角度的估计精度均为0.1且互相独立
        // 那么协方差则为对角为0.01的矩阵，信息阵则为100的矩阵
        information(0,0) = information(1,1) = information(2,2) = 100;
        information(3,3) = information(4,4) = information(5,5) = 100;
        // 也可以将角度设大一些，表示对角度的估计更加准确
        edge->setInformation( information );
        // 边的估计即是pnp求解之结果
        Eigen::Isometry3d T = cvMat2Eigen( result.rvec, result.tvec );
        edge->setMeasurement( T.inverse() );
        // 将此边加入图中
        opti.addEdge(edge);

        return true;
    }
    void checkNearbyLoops(int src_id, g2o::SparseOptimizer& opti) {
        for (int i = max(0,src_id-NEARBY_LOOPS-1); i < src_id-1; i++) {
            checkFrameMatch(src_id, i, opti);
        }
    }
    void checkRandomLoops(int src_id, g2o::SparseOptimizer& opti) {
        // do nothing temperory

        //srand( (unsigned int) time(NULL) );
        //for (int i = 0; i < checkNearbyLoops; i++)
    }

};


double gettime(struct timeval &tt1, struct timeval &tt2);

#define THREAD_SPIN 1


#endif // SLAM_H
