#include "colorfactor.hpp"
#include <stdio.h>
#include <opencv/highgui.h>

namespace ya_imagekit {
  int Image::read(const char * filename) {
    using namespace cv;

    rgb = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);    
    if (rgb == NULL) {printf("Image NOT loaded!\n"); return -1;}

    if (std::string(rgb->colorModel).compare("RGB") != 0) 
      {printf("Image NOT in RGB!"); return -1;};


    // (to implement) check if it is a color image 
    // ...

    // display THE image 

    imshow("original image", Mat(rgb));
    printf("Press a button to continue ..."); fflush(stdout); waitKey(0);
    printf("[done]\n");


    // convert to RGB to Lab space
    
    lab = Mat(rgb->height, rgb->width, CV_8UC3);
    cvtColor(Mat(rgb), lab, CV_RGB2Lab);

    return 0;
  }

  int Image::createSegmentsByKmeans(int k) {
    using namespace cv;
    // allocate segmentation map
    
    // convert to data array
    Mat pixels, pixels_array, segments_array;
    lab.convertTo(pixels, CV_32FC1);
    segments_array = Mat(lab.rows * lab.cols, 1, CV_32SC1);
    pixels_array = pixels.reshape(1, lab.rows*lab.cols);
    pixels_array.col(0) = .1 * pixels_array.col(0);//scale illuminance 
    

    kmeans(pixels_array, k, segments_array,
	   cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 50, 1.0),
	   3, KMEANS_RANDOM_CENTERS);

    segments_map = segments_array.reshape(0, lab.rows);    

    printf("Lab kmeans segmentation finished!\n");
    return 0;
  }

  int Image::prepareSegments() {
    using namespace cv;
    if (segments_map.empty()) {printf("Empty Segmentation Map"); return -1;}

    double minVal, maxVal;
    minMaxLoc(segments_map, &minVal, &maxVal);

    for (int i=0; i <= maxVal; ++i) {
      UnarySeg s; 
      s.label=i;s.area=0;
      s.avgLab[0] = s.avgLab[1] = s.avgLab[2] =0;
      s.avgRGB[0] = s.avgRGB[1] = s.avgRGB[2] =0;
      usegs.push_back(s);
    }

    Mat rgb_array = Mat(rgb);

    for (int i=0; i<segments_map.cols*segments_map.rows; ++i) {
      int idx = segments_map.at<int> (i);
      Vec3b pixel = rgb_array.at<Vec3b>(i);

      usegs[idx].avgRGB[0] += pixel[0];
      usegs[idx].avgRGB[1] += pixel[1];
      usegs[idx].avgRGB[2] += pixel[2];

      // Lab to be added ...

      usegs[idx].area ++;
    }

    for (int i=0; i<usegs.size(); ++i) {
      usegs[i].avgRGB[0] /= usegs[i].area;
      usegs[i].avgRGB[1] /= usegs[i].area;
      usegs[i].avgRGB[2] /= usegs[i].area;      

      usegs[i].avgLab[0] /= usegs[i].area;
      usegs[i].avgLab[1] /= usegs[i].area;
      usegs[i].avgLab[2] /= usegs[i].area;

    }

    return 0;
  }

  int Image::displaySegments() {
    using namespace cv;
    if (usegs.size() == 0 ) {printf("No Segments!\n"); return -1;}

    Mat rgb_array = Mat(rgb, true);

    for (int i=0; i<segments_map.cols*segments_map.rows; ++i) {
      int idx = segments_map.at<int> (i);
      Vec3b &pixel = rgb_array.at<Vec3b>(i);
      pixel[0] = std::floor (usegs[idx].avgRGB[0]);
      pixel[1] = std::floor (usegs[idx].avgRGB[1]);
      pixel[2] = std::floor (usegs[idx].avgRGB[2]);
      
    }

    imshow("segmentation map", rgb_array);
    printf("Press a button to continue ..."); fflush(stdout); waitKey(0);
    printf("[done]\n");

    return 0;
  }

}
